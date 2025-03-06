/*
* FAT Driver Module
* Author: Ducson9112k (bản gốc) – Đã chỉnh sửa và tối ưu bởi ChatGPT
* Description: Module xử lý FAT12/FAT16/FAT32 từ file ảnh FAT (ví dụ: "floppy.img")
*
* Chức năng chính:
*   - Khởi tạo driver và HAL.
*   - Đọc boot sector và giải mã các tham số hệ thống tập tin.
*   - Xây dựng cây thư mục (dạng danh sách liên kết) từ dữ liệu trên đĩa.
*   - Liệt kê nội dung thư mục theo cây (hỗ trợ duyệt các thư mục con).
*   - Đọc dữ liệu file theo chuỗi cluster.
*   - Kiểm tra sự tồn tại của thư mục.
*
* Lưu ý: Hàm fat_driver_get_next_cluster sử dụng kiểu tham số uint32_t.
*/

#include "hal.h"
#include "fat_driver.h"
#include "linkedlist.h"

/*---------------------------------------------------------------------
*                GLOBAL VARIABLES
*---------------------------------------------------------------------*/
static FATFS_BootData_t boot_data;
static FATFS_FatTypes_t fat_type;
static bool is_boot_startup = false;

/*---------------------------------------------------------------------
*                INTERNAL TYPE DEFINITIONS
*---------------------------------------------------------------------*/
/*
* Cấu trúc đại diện cho một nút trong cây thư mục FAT.
* Mỗi nút lưu trữ tên, thông tin file (nếu là file), con trỏ đến nút cha và danh sách các nút con.
*/
typedef struct FATFS_Node {
    char name[256];              /* Tên file hoặc thư mục */
    int isDirectory;             /* 1 nếu là thư mục, 0 nếu là file */
    FAT_DRIVER_FileInfo fileInfo;/* Thông tin file (nếu có) */
    struct FATFS_Node *parent;   /* Con trỏ đến nút cha (NULL nếu là root) */
    linkedlist_t *children;      /* Danh sách liên kết chứa các FATFS_Node* của các nút con */
} FATFS_Node;

/*---------------------------------------------------------------------
*         INTERNAL HELPER FUNCTION PROTOTYPES
*---------------------------------------------------------------------*/
static inline uint32_t cluster_to_sector(uint32_t cluster);
static int is_end_of_chain(uint32_t cluster);
static void rtrim_spaces(char *str);
static int starts_with_token(const char *entry, const char *token);
static int32_t reverse_max_4byte(const uint8_t *byte, uint32_t count);
static int read_cluster_chain(uint32_t start_cluster);
static void format_sfn_name(const uint8_t *name, const uint8_t *ext, char *output);
static void fat_driver_extract_file_info(const FATFS_EntryFormat_t *entry, FAT_DRIVER_FileInfo *info);
static FATFS_Node *find_node_by_path(linkedlist_t *root_list, const char *path);

/* Prototype của hàm output_data (sẽ được định nghĩa ở cuối file) */
static void output_data(const unsigned char *data, uint32_t size);

/*---------------------------------------------------------------------
*                      INTERNAL HELPER FUNCTIONS
*---------------------------------------------------------------------*/

/*
* cluster_to_sector:
*   Chuyển đổi số cluster sang số sector tương ứng.
*   Theo chuẩn FAT, số cluster bắt đầu từ 2.
*/
static inline uint32_t cluster_to_sector(uint32_t cluster) {
    return boot_data.FirstDataClus + ((cluster - 2) * boot_data.SectorPerClus);
}

/*
* is_end_of_chain:
*   Kiểm tra xem cluster có đánh dấu kết thúc chuỗi hay không.
*/
static int is_end_of_chain(uint32_t cluster) {
    return (cluster >= FAT_DRIVER_12_LAST_CLUSTER_START) ? 1 : 0;
}

/*
* rtrim_spaces:
*   Loại bỏ các khoảng trắng thừa ở cuối chuỗi.
*/
static void rtrim_spaces(char *str) {
    char *end = str + strlen(str) - 1;
    while (end >= str && ((*end == ' ') || (*end == '\0'))) {
        *end = '\0';
        end--;
    }
}

/*
* starts_with_token:
*   Kiểm tra xem chuỗi entry có bắt đầu bằng token cho trước và ký tự sau token là dấu kết thúc hoặc khoảng trắng.
*/
static int starts_with_token(const char *entry, const char *token) {
    size_t token_len = strlen(token);
    if (strncmp(entry, token, token_len) != 0)
        return 0;
    char next_char = entry[token_len];
    return ((next_char == '\0') || isspace(next_char) || ispunct(next_char)) ? 1 : 0;
}

/*
* reverse_max_4byte:
*   Chuyển đổi tối đa 4 byte từ mảng byte thành số nguyên 32-bit theo thứ tự ngược lại.
*/
static int32_t reverse_max_4byte(const uint8_t *byte, uint32_t count) {
    int32_t result = 0;
    if (count <= 4) {
        for (int i = count - 1; i >= 0; i--) {
            result = (result << 8) | byte[i];
        }
    }
    return result;
}

/*
* read_cluster_chain:
*   Đọc toàn bộ chuỗi cluster bắt đầu từ cluster được cho và xuất dữ liệu qua hàm output_data.
*/
static int read_cluster_chain(uint32_t start_cluster) {
    unsigned char cluster_buffer[FAT_DRIVER_BYTES_PER_SECTOR];
    uint32_t current_cluster = start_cluster;
    
    while (!is_end_of_chain(current_cluster)) {
        uint32_t data_sector = cluster_to_sector(current_cluster);
        for (uint32_t sector_offset = 0; sector_offset < boot_data.SectorPerClus; sector_offset++) {
            if (hal_read_sector(data_sector + sector_offset, cluster_buffer) != 0)
                return -1;
            output_data(cluster_buffer, FAT_DRIVER_BYTES_PER_SECTOR);
        }
        current_cluster = fat_driver_get_next_cluster(current_cluster);
    }
    return 0;
}

/*
* format_sfn_name:
*   Định dạng tên file kiểu SFN bằng cách kết hợp phần tên và phần mở rộng.
*/
static void format_sfn_name(const uint8_t *name, const uint8_t *ext, char *output) {
    memcpy(output, name, FAT_DRIVER_SFN_NAME_PART);
    output[FAT_DRIVER_SFN_NAME_PART] = '\0';
    rtrim_spaces(output);
    if (ext[0] != ' ') {
        strcat(output, ".");
        memcpy(output + strlen(output), ext, FAT_DRIVER_SFN_EXT_LENGTH);
        output[strlen(output) + FAT_DRIVER_SFN_EXT_LENGTH] = '\0';
        rtrim_spaces(output + strlen(output) - FAT_DRIVER_SFN_EXT_LENGTH);
    }
}

/*
* fat_driver_extract_file_info:
*   Trích xuất và giải mã thông tin file từ một entry của FAT vào cấu trúc FAT_DRIVER_FileInfo.
*/
static void fat_driver_extract_file_info(const FATFS_EntryFormat_t *entry, FAT_DRIVER_FileInfo *info) {
    info->attributes = entry->DIR_Attr[0];
    const uint16_t raw_crt_time = reverse_max_4byte(entry->DIR_CrtTime, sizeof(entry->DIR_CrtTime));
    const uint16_t raw_crt_date = reverse_max_4byte(entry->DIR_CrtDate, sizeof(entry->DIR_CrtDate));
    const uint16_t raw_wrt_time = reverse_max_4byte(entry->DIR_WrtTime, sizeof(entry->DIR_WrtTime));
    
    info->crtTime.hour = FAT_DRIVER_EXTRACT_HOURS(raw_crt_time);
    info->crtTime.minute = FAT_DRIVER_EXTRACT_MINUTES(raw_crt_time);
    info->crtTime.second = FAT_DRIVER_EXTRACT_SECONDS(raw_crt_time);
    info->crtTime.day = FAT_DRIVER_EXTRACT_DAY(raw_crt_date);
    info->crtTime.month = FAT_DRIVER_EXTRACT_MONTH(raw_crt_date);
    info->crtTime.year = FAT_DRIVER_EXTRACT_YEAR(raw_crt_date) + BIN_YEAR_OFFSET_FROM;
    
    info->wrtTime.hour = FAT_DRIVER_EXTRACT_HOURS(raw_wrt_time);
    info->wrtTime.minute = FAT_DRIVER_EXTRACT_MINUTES(raw_wrt_time);
    info->wrtTime.second = FAT_DRIVER_EXTRACT_SECONDS(raw_wrt_time);
    
    const uint16_t fst_clus_hi = reverse_max_4byte(entry->DIR_FstClusHI, sizeof(entry->DIR_FstClusHI));
    const uint16_t fst_clus_lo = reverse_max_4byte(entry->DIR_FstClusLO, sizeof(entry->DIR_FstClusLO));
    info->firstCluster = ((uint32_t)fst_clus_hi << 16) | fst_clus_lo;
    info->fileSize = reverse_max_4byte(entry->DIR_FileSize, sizeof(entry->DIR_FileSize));
}

/*
* find_node_by_path:
*   Tìm kiếm đệ quy trong cây thư mục theo đường dẫn (ví dụ: "/", "/subdir", "/subdir/nested").
*   Nếu root_list không NULL, sử dụng danh sách chứa nút root; nếu NULL, giả định nút root đã có sẵn.
*/
static FATFS_Node *find_node_by_path(linkedlist_t *root_list, const char *path) {
    if (strcmp(path, "/") == 0) {
        if (root_list)
            return (FATFS_Node *)llist_get_first(root_list);
        else
            return NULL;
    }
    char path_copy[256];
    strcpy(path_copy, path);
    char *token = strtok(path_copy, "/");
    FATFS_Node *current = (root_list) ? (FATFS_Node *)llist_get_first(root_list) : NULL;
    if (current == NULL)
        return NULL;
    if (current->children == NULL)
        return NULL;
    current = (FATFS_Node *)llist_get_first(current->children);
    while (token != NULL && current != NULL) {
        FATFS_Node *found = NULL;
        linkedlist_iterator_t it;
        llist_iterator_init(current->parent->children, &it);
        while (llist_iterator_has_next(&it)) {
            FATFS_Node *node = (FATFS_Node *)llist_iterator_next(&it);
            if (strcmp(node->name, token) == 0) {
                found = node;
                break;
            }
        }
        if (found == NULL)
            return NULL;
        current = found;
        token = strtok(NULL, "/");
        if (token != NULL) {
            if (current->children == NULL)
                return NULL;
            current = (FATFS_Node *)llist_get_first(current->children);
        }
    }
    return current;
}

/*---------------------------------------------------------------------
*                     PUBLIC API FUNCTIONS
*---------------------------------------------------------------------*/

/**
 * @brief Khởi tạo FAT Driver bằng cách khởi tạo HAL, đọc boot sector và xây dựng cây thư mục.
 *
 * @param img_path Đường dẫn tới file ảnh FAT (ví dụ: "floppy.img").
 * @param file_list Con trỏ đến danh sách liên kết để lưu trữ cây thư mục.
 * @return 0 nếu thành công, -1 nếu có lỗi.
 */
int fat_driver_init(const char *img_path, linkedlist_t *file_list) {
    if (hal_init(img_path) != 0) {
        printf("Error: Cannot initialize HAL.\n");
        return -1;
    }
    if (fat_driver_read_boot_sector() == -1) {
        print_colored("Error: Failed to read boot sector.\n", ANSI_COLOR_RED, ANSI_BG_BLACK);
        return -1;
    }
    
    /* Tạo nút root và thêm vào file_list */
    FATFS_Node root;
    memset(&root, 0, sizeof(FATFS_Node));
    strcpy(root.name, "root");
    root.isDirectory = 1;
    root.parent = NULL;
    root.children = llist_init();
    llist_add(file_list, &root, sizeof(FATFS_Node));
    
    /* Xây dựng cây thư mục từ đĩa với đường dẫn "/" */
    is_boot_startup = true;
    fat_driver_list_directory("/", file_list);
    is_boot_startup = false;
    return 0;
}

/**
 * @brief Đọc boot sector từ đĩa và giải mã các tham số hệ thống tập tin.
 *
 * @return 0 nếu thành công, -1 nếu có lỗi.
 */
int fat_driver_read_boot_sector(void) {
    unsigned char buffer[FAT_DRIVER_BYTES_PER_SECTOR];
    if (hal_read_sector(0, buffer) != 0) {
        printf("Error: Cannot read boot sector.\n");
        return -1;
    }
    /* Ép kiểu buffer thành cấu trúc boot sector FAT12/16 */
    FATFS_BootFormat126_t *boot_sector = (FATFS_BootFormat126_t *)buffer;
    boot_data.BytePerSec    = *(uint16_t *)boot_sector->BPB_BytsPerSec;
    boot_data.SectorPerClus = boot_sector->BPB_SecPerClus[0];
    boot_data.RsvdSecCnt    = *(uint16_t *)boot_sector->BPB_RsvdSecCnt;
    boot_data.NumFats       = boot_sector->BPB_NumFATs[0];
    boot_data.RootEntCnt    = *(uint16_t *)boot_sector->BPB_RootEntCnt;
    boot_data.TotSec        = *(uint16_t *)boot_sector->BPB_TotSec16;
    if (boot_data.TotSec == 0)
        boot_data.TotSec = *(uint32_t *)boot_sector->BPB_TotSec32;
    boot_data.FatSz = *(uint16_t *)boot_sector->BPB_FATSz16;
    if (boot_data.FatSz == 0)
        boot_data.FatSz = *(uint32_t *)((FATFS_BootFormat32_t *)boot_sector)->BPB_FATSz32;
    
    /* Tính toán các tham số hệ thống */
    uint32_t root_dir_sectors = ((boot_data.RootEntCnt * DIR_NUMBER_OF_BYTES_PER_ENTRY) + (boot_data.BytePerSec - 1)) / boot_data.BytePerSec;
    uint32_t first_data_sector = boot_data.RsvdSecCnt + (boot_data.NumFats * boot_data.FatSz) + root_dir_sectors;
    uint32_t total_data_sectors = boot_data.TotSec - first_data_sector;
    uint32_t total_clusters = total_data_sectors / boot_data.SectorPerClus;
    if (total_clusters < FAT_DRIVER_MAX_CLUSTER_OF_FATFS_12)
        fat_type = FAT12;
    else if (total_clusters < FAT_DRIVER_MAX_CLUSTER_OF_FATFS_16)
        fat_type = FAT16;
    else
        fat_type = FAT32;
    
    boot_data.RootDirSector = boot_data.RsvdSecCnt + (boot_data.NumFats * boot_data.FatSz);
    boot_data.FirstRootClus = boot_data.RootDirSector;
    boot_data.FirstDataClus = first_data_sector;
    return 0;
}

/**
 * @brief Liệt kê các mục nhập trong thư mục theo đường dẫn.
 *
 * Nếu đường dẫn là "/" thì đọc từ đĩa và cập nhật danh sách con của nút root;
 * nếu không, tìm kiếm nút tương ứng trong cây và hiển thị các nút con.
 *
 * @param path Đường dẫn của thư mục cần liệt kê.
 * @param file_list Danh sách liên kết chứa cây thư mục.
 * @return 0 nếu thành công, -1 nếu có lỗi.
 */
int fat_driver_list_directory(const char *path, linkedlist_t *file_list) {
    if (path == NULL || strlen(path) == 0 || strcmp(path, "/") == 0) {
        uint32_t root_dir_sectors = ((boot_data.RootEntCnt * DIR_NUMBER_OF_BYTES_PER_ENTRY) + (boot_data.BytePerSec - 1)) / boot_data.BytePerSec;
        uint32_t start_sector = boot_data.RootDirSector;
        unsigned char sector_buffer[FAT_DRIVER_BYTES_PER_SECTOR];
        FATFS_Node *root = (FATFS_Node *)llist_get_first(file_list);
        if (root == NULL) {
            print_colored("Error: Root node not found in list.\n", ANSI_COLOR_RED, ANSI_BG_BLACK);
            return -1;
        }
        if (root->children == NULL)
            root->children = llist_init();
        for (uint32_t sec_offset = 0; sec_offset < root_dir_sectors; sec_offset++) {
            if (hal_read_sector(start_sector + sec_offset, sector_buffer) != 0) {
                print_colored("Error: Cannot read root directory sector.\n", ANSI_COLOR_RED, ANSI_BG_BLACK);
                return -1;
            }
            for (uint32_t entry_idx = 0; entry_idx < FAT_DRIVER_ENTRYS_OF_SECTOR; entry_idx++) {
                FATFS_EntryFormat_t *entry = (FATFS_EntryFormat_t *)(sector_buffer + entry_idx * DIR_NUMBER_OF_BYTES_PER_ENTRY);
                if (entry->DIR_Name[0] == FAT_DRIVER_FILENAME_NEVER_USED)
                    break;
                if ((entry->DIR_Name[0] == FAT_DRIVER_FILENAME_DELETED_1) ||
                    (entry->DIR_Name[0] == FAT_DRIVER_FILENAME_DELETED_2) ||
                    (entry->DIR_Attr[0] & FAT_DRIVER_ATTR_VOLUME_LABEL))
                    continue;
                FATFS_Node *node = (FATFS_Node *)malloc(sizeof(FATFS_Node));
                if (!node)
                    continue;
                memset(node, 0, sizeof(FATFS_Node));
                format_sfn_name(entry->DIR_Name, entry->DIR_Ext, node->name);
                fat_driver_extract_file_info(entry, &node->fileInfo);
                node->isDirectory = (entry->DIR_Attr[0] & FAT_DRIVER_ATTR_DIRECTORY) ? 1 : 0;
                node->parent = root;
                node->children = (node->isDirectory) ? llist_init() : NULL;
                llist_add(root->children, node, sizeof(FATFS_Node));
                if (!is_boot_startup)
                {
                    if (node->isDirectory)
                        print_colored(node->name, ANSI_COLOR_CYAN, ANSI_BG_BLACK);
                    else
                        print_colored(node->name, ANSI_COLOR_WHITE, ANSI_BG_BLACK);
                    printf("\n");
                }
            }
        }
        return 0;
    } else {
        FATFS_Node *parent = find_node_by_path(file_list, path);
        if (parent == NULL) {
            print_colored("Directory not found\n", ANSI_COLOR_RED, ANSI_BG_BLACK);
            return -1;
        }
        if (parent->children == NULL) {
            print_colored("Directory is empty\n", ANSI_COLOR_YELLOW, ANSI_BG_BLACK);
            return 0;
        }
        linkedlist_iterator_t it;
        llist_iterator_init(parent->children, &it);
        while (llist_iterator_has_next(&it)) {
            FATFS_Node *child = (FATFS_Node *)llist_iterator_next(&it);
            if (child->isDirectory)
                print_colored(child->name, ANSI_COLOR_CYAN, ANSI_BG_BLACK);
            else
                print_colored(child->name, ANSI_COLOR_WHITE, ANSI_BG_BLACK);
            printf("\n");
        }
        return 0;
    }
}

/**
 * @brief Tìm file theo đường dẫn đầy đủ (ví dụ: "/subdir/file.txt") và xuất nội dung file.
 *
 * @param full_path Đường dẫn đầy đủ của file.
 * @param file_list Danh sách liên kết chứa cây thư mục.
 * @return 0 nếu thành công, -1 nếu có lỗi.
 */
int fat_driver_read_file(const char *full_path, linkedlist_t *file_list) {
    char parent_path[256] = {0};
    char filename[256] = {0};
    const char *last_slash = strrchr(full_path, '/');
    if (last_slash == NULL) {
        strcpy(filename, full_path);
        strcpy(parent_path, "/");
    } else {
        size_t parent_len = last_slash - full_path;
        strncpy(parent_path, full_path, parent_len);
        parent_path[parent_len] = '\0';
        strcpy(filename, last_slash + 1);
        if (strlen(parent_path) == 0)
            strcpy(parent_path, "/");
    }
    FATFS_Node *parent = find_node_by_path(file_list, parent_path);
    if (parent == NULL || parent->children == NULL) {
        print_colored("Directory not found\n", ANSI_COLOR_RED, ANSI_BG_BLACK);
        return -1;
    }
    linkedlist_iterator_t it;
    llist_iterator_init(parent->children, &it);
    while (llist_iterator_has_next(&it)) {
        FATFS_Node *node = (FATFS_Node *)llist_iterator_next(&it);
        if (!node->isDirectory && starts_with_token(node->name, filename) == 0) {
            uint32_t start_cluster = node->fileInfo.firstCluster;
            return read_cluster_chain(start_cluster);
        }
    }
    print_colored("File not found\n", ANSI_COLOR_YELLOW, ANSI_BG_BLACK);
    return -1;
}

/**
 * @brief Kiểm tra sự tồn tại của thư mục theo đường dẫn trong cây.
 *
 * @param path Đường dẫn của thư mục.
 * @return 1 nếu tồn tại, 0 nếu không.
 */
int fat_driver_directory_exists(const char *path) {
    FATFS_Node *node = find_node_by_path(NULL, path);
    return (node != NULL && node->isDirectory) ? 1 : 0;
}

/**
 * @brief Lấy cluster tiếp theo từ FAT dựa trên cluster hiện tại.
 *
 * @param current_cluster Số cluster hiện tại.
 * @return Số cluster tiếp theo, hoặc -1 nếu lỗi hoặc kết thúc chuỗi.
 */
int fat_driver_get_next_cluster(uint32_t current_cluster) {
    uint32_t fat_start_sector = boot_data.RsvdSecCnt;
    unsigned char buffer[FAT_DRIVER_BYTES_PER_SECTOR];

    if (fat_type == FAT12) {
        uint32_t fat_offset = (current_cluster * 3) / 2;
        uint32_t sector = fat_start_sector + (fat_offset / boot_data.BytePerSec);
        uint16_t offset = fat_offset % boot_data.BytePerSec;
        if (hal_read_sector(sector, buffer) != 0) {
            printf("Error: Cannot read sector %u.\n", sector);
            return -1;
        }
        uint16_t entry_value;
        memcpy(&entry_value, buffer + offset, 2);
        if ((current_cluster % 2) == 0)
            entry_value &= FAT_DRIVER_12_LAST_CLUSTER_END;
        else
            entry_value = (entry_value >> 4) & FAT_DRIVER_12_LAST_CLUSTER_END;
        if (entry_value >= FAT_DRIVER_12_LAST_CLUSTER_START)
            return -1;
        if (entry_value == FAT_DRIVER_12_BAD_CLUSTER) {
            printf("Damaged cluster: %u\n", current_cluster);
            return -1;
        }
        return entry_value;
    } else if (fat_type == FAT16) {
        uint32_t fat_offset = current_cluster * 2;
        uint32_t sector = fat_start_sector + (fat_offset / boot_data.BytePerSec);
        uint16_t offset = fat_offset % boot_data.BytePerSec;
        if (hal_read_sector(sector, buffer) != 0) {
            printf("Error: Cannot read sector %u.\n", sector);
            return -1;
        }
        uint16_t entry_value;
        memcpy(&entry_value, buffer + offset, 2);
        if (entry_value >= 0xFFF8)
            return -1;
        if (entry_value == 0xFFF7) {
            printf("Damaged cluster: %u\n", current_cluster);
            return -1;
        }
        return entry_value;
    } else if (fat_type == FAT32) {
        uint32_t fat_offset = current_cluster * 4;
        uint32_t sector = fat_start_sector + (fat_offset / boot_data.BytePerSec);
        uint16_t offset = fat_offset % boot_data.BytePerSec;
        if (hal_read_sector(sector, buffer) != 0) {
            printf("Error: Cannot read sector %u.\n", sector);
            return -1;
        }
        uint32_t entry_value;
        memcpy(&entry_value, buffer + offset, 4);
        entry_value &= 0x0FFFFFFF;
        if (entry_value >= 0x0FFFFFF8)
            return -1;
        if (entry_value == 0x0FFFFFF7) {
            printf("Damaged cluster: %u\n", current_cluster);
            return -1;
        }
        return entry_value;
    }
    return -1;
}

static void output_data(const unsigned char *data, uint32_t size) {
    fwrite(data, 1, size, stdout);
    printf("\n");
}

/*---------------------------------------------------------------------
*                     END OF FAT DRIVER MODULE
*---------------------------------------------------------------------*/
