/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 *********************************************************************/

/*********************************************************************
 * Include
 *********************************************************************/
#include <ctype.h>
#include "hal.h"
#include "fat_driver.h"

/*********************************************************************
 * Define
 *********************************************************************/
/**
 * Global variables for boot sector information and FAT type
 */
static FATFS_BootData_t boot_data;
static FATFS_FatTypes_t fat_type;

/*********************************************************************
 * Function prototypes
 *********************************************************************/
static int starts_with_token(const char *entry, const char *token);
static int32_t reverseMax4Byte(const uint8_t *byte, uint32_t count);

/**
 * Initialize FAT Driver
 * @param img_path Path to the FAT image file
 * @return 0 on success, -1 on failure
 */
int fat_driver_init(const char *img_path) {
    // Initialize HAL
    if (hal_init(img_path) != 0) {
        printf("Error: Cannot initialize HAL.\n");
        return -1;
    }

    // Read boot sector
    unsigned char buffer[FAT_DRIVER_BYTES_PER_SECTOR];
    if (hal_read_sector(0, buffer) != 0) {
        printf("Error: Cannot read boot sector.\n");
        return -1;
    }

    // Extract information from boot sector
    FATFS_BootFormat126_t *boot_sector = (FATFS_BootFormat126_t *)buffer;
    boot_data.BytePerSec = *(uint16_t *)boot_sector->BPB_BytsPerSec;
    boot_data.SectorPerClus = boot_sector->BPB_SecPerClus[0];
    boot_data.RsvdSecCnt = *(uint16_t *)boot_sector->BPB_RsvdSecCnt;
    boot_data.NumFats = boot_sector->BPB_NumFATs[0];
    boot_data.RootEntCnt = *(uint16_t *)boot_sector->BPB_RootEntCnt;
    boot_data.TotSec = *(uint16_t *)boot_sector->BPB_TotSec16;
    if (boot_data.TotSec == 0) {
        boot_data.TotSec = *(uint32_t *)boot_sector->BPB_TotSec32;
    }
    boot_data.FatSz = *(uint16_t *)boot_sector->BPB_FATSz16;
    if (boot_data.FatSz == 0) {
        boot_data.FatSz = *(uint32_t *)((FATFS_BootFormat32_t *)boot_sector)->BPB_FATSz32;
    }

    // Calculate necessary parameters
    uint32_t root_dir_sectors = ((boot_data.RootEntCnt * 32) + (boot_data.BytePerSec - 1)) / boot_data.BytePerSec;
    uint32_t first_data_sector = boot_data.RsvdSecCnt + (boot_data.NumFats * boot_data.FatSz) + root_dir_sectors;
    uint32_t total_data_sectors = boot_data.TotSec - first_data_sector;
    uint32_t total_clusters = total_data_sectors / boot_data.SectorPerClus;

    // Determine FAT type
    if (total_clusters < FAT_DRIVER_MAX_CLUSTER_OF_FATFS_12) {
        fat_type = FAT12;
    } else if (total_clusters < FAT_DRIVER_MAX_CLUSTER_OF_FATFS_16) {
        fat_type = FAT16;
    } else {
        fat_type = FAT32;
    }

    // Store additional information
    boot_data.RootDirSector = boot_data.RsvdSecCnt + (boot_data.NumFats * boot_data.FatSz);
    boot_data.FirstDataClus = first_data_sector;

    return 0;
}

/**
 * Read boot sector (integrated into init)
 * @return 0 on success, -1 on failure
 */
int fat_driver_read_boot_sector() {
    // Integrated into fat_driver_init, no separate implementation needed
    
    return 0;
}

/**
 * Liệt kê nội dung của thư mục tại path
 * @param path Đường dẫn của thư mục cần liệt kê
 * @return 0 nếu thành công, -1 nếu thất bại
 */
int fat_driver_list_directory(const char *path) {
    // In thông báo đang liệt kê thư mục
    // printf("Listing files/directories at %s:\n", path);

    // Giả định bắt đầu từ root directory (cần phân tích path để hỗ trợ thư mục con)
    uint32_t sector = boot_data.RootDirSector;
    unsigned char buffer[FAT_DRIVER_BYTES_PER_SECTOR];

    // Đọc sector chứa thư mục
    if (hal_read_sector(sector, buffer) != 0) {
        print_colored("Error: Cannot read root directory.\n", ANSI_COLOR_RED, ANSI_BG_BLACK);
        return -1;
    }

    // Duyệt qua các entry trong sector
    for (uint32_t i = 0; i < FAT_DRIVER_ENTRYS_OF_SECTOR; i++) {
        uint8_t *entry = buffer + i * DIR_NUMBER_OF_BYTES_PER_ENTRY;

        // Kiểm tra entry hợp lệ
        if (entry[0] == FAT_DRIVER_FILENAME_NEVER_USED) {
            break;  // Không còn entry nào nữa
        }
        if (entry[0] == FAT_DRIVER_FILENAME_DELETED_1 || entry[0] == FAT_DRIVER_FILENAME_DELETED_2) {
            continue;  // Bỏ qua entry đã xóa
        }
        if (entry[0] == FAT_DRIVER_FILENAME_SPECIAL_ENTRY) {
            continue;  // Bỏ qua các entry đặc biệt như '.' và '..'
        }

        // Lấy thuộc tính của entry (offset 11)
        uint8_t attr = entry[11];

        // Bỏ qua entry LFN
        if ((attr & ATT_LONG_FILE_NAME) == ATT_LONG_FILE_NAME) {
            continue;  // Không in entry LFN
        }

        // Trích xuất tên file (SFN - Short File Name)
        char filename[13];  // 8.3 format + null terminator
        memcpy(filename, entry, 11);
        filename[11] = '\0';  // Kết thúc chuỗi

        // Xử lý khoảng trắng trong tên file
        for (int j = 7; j >= 0; j--) {
            if (filename[j] == ' ') {
                filename[j] = '\0';
            } else {
                break;
            }
        }
        if (filename[8] != ' ') {
            strcat(filename, ".");
            strncat(filename, entry + 8, 3);
        }

        // Kiểm tra và in entry
        if (attr & ATT_DIRECTORY) {
            // In thư mục với màu xanh dương
            print_colored(filename, ANSI_COLOR_BLUE, ANSI_BG_BLACK);
            printf("\n");
        } else {
            // In file với màu trắng (mặc định)
            printf("%s\n", filename);
        }
    }
    return 0;
}

/**
 * Read file (placeholder)
 * @param filename Path of the file to read
 * @return 0 on success, -1 on failure
 */
int fat_driver_read_file(const char *filename) {
    // TODO: Find file, read cluster chain, return content
    // Find file in root directory (need to support subdirectories)
    uint32_t sector = boot_data.RootDirSector;
    unsigned char buffer[FAT_DRIVER_BYTES_PER_SECTOR];

    // Đọc sector chứa thư mục
    if (hal_read_sector(sector, buffer) != 0) {
        print_colored("Error: Cannot read root directory.\n", ANSI_COLOR_RED, ANSI_BG_BLACK);
        return -1;
    }

    // Duyệt qua các entry trong sector
    for (uint32_t i = 0; i < FAT_DRIVER_ENTRYS_OF_SECTOR; i++) {
        FATFS_EntryFormat_t *entry = (FATFS_EntryFormat_t *)(buffer + i * DIR_NUMBER_OF_BYTES_PER_ENTRY);

        /*  #define FAT_DRIVER_FILENAME_NEVER_USED 0x00U
 #define FAT_DRIVER_FILENAME_DELETED_1 0xE5U
 #define FAT_DRIVER_FILENAME_DELETED_2 0x05U
 #define FAT_DRIVER_FILENAME_SPECIAL_ENTRY 0x2EU */
        // Kiểm tra entry hợp lệ
        if (entry->uint8_t[0] == FAT_DRIVER_FILENAME_NEVER_USED) {
            break;  // Không còn entry nào nữa
        }
        if (entry->DIR_Attr[0] == ATTR_DIRECTORY){
            break;
        }
        if ((char *)entry == FAT_DRIVER_FILENAME_DELETED_1 || (char *)entry == FAT_DRIVER_FILENAME_DELETED_2) {
            continue;  // Bỏ qua entry đã xóa
        }
        if ((char *)entry == FAT_DRIVER_FILENAME_SPECIAL_ENTRY) {
            continue;  // Bỏ qua các entry đặc biệt như '.' và '..'
        }

        /* loại bỏ \r\n trong filename */
        char tmp[DIR_NAME_LEN];
        strncpy(tmp, filename, DIR_NAME_LEN);
        tmp[DIR_NAME_LEN-1] = '\0'; // kết thúc chuỗi
        
        // So sánh với tên file đã cho
        if (starts_with_token((const char *)entry, (const char *)filename)) {
            // Đọc cluster chain của file
            uint32_t cluster = reverseMax4Byte(entry->DIR_FstClusLO, DIR_FST_CLUS_LOW_LEN);
            // uint32_t cluster = *(uint16_t *)entry + 26;
            while (cluster < 0xFF8) {
                // Đọc sector của cluster
                uint32_t sector = boot_data.FirstDataClus + (cluster - 2) * boot_data.SectorPerClus;

                // Read each sector in the cluster and copy to buffer
                for (uint32_t i = 0; i < boot_data.SectorPerClus; i++)
                {
                    if (hal_read_sector(sector, buffer) != 0) {
                        printf("Error: Cannot read sector %u.\n", cluster);
                        return -1;
                    }

                    // In nội dung của sector
                    for (uint32_t j = 0; j < FAT_DRIVER_BYTES_PER_SECTOR; j++) {
                        printf("%c", buffer[j]);
                    }
                }

                // Tìm cluster tiếp theo
                cluster = fat_driver_get_next_cluster(cluster);
            }
            printf("\n");
            return 0;
        }
    }
    return 0;
}

/**
 * Check if directory exists (placeholder)
 * @param path Path of the directory to check
 * @return 1 if directory exists, 0 otherwise
 */
int fat_driver_directory_exists(const char *path) {
    // TODO: Check if path is a valid directory
    // Assume the function to read a sector and check directory entry is available.
    unsigned char buffer[FAT_DRIVER_BYTES_PER_SECTOR];
    uint32_t sector = boot_data.RootDirSector; // Start from root directory for simplicity

    if (hal_read_sector(sector, buffer) != 0) {
        printf("Error: Cannot read sector %u.\n", sector);
        return 0;
    }

    // Iterate through directory entries in the sector
    for (uint32_t i = 0; i < FAT_DRIVER_ENTRYS_OF_SECTOR; i++) {
        uint8_t *entry = buffer + i * DIR_NUMBER_OF_BYTES_PER_ENTRY;
        if (entry[0] == FAT_DRIVER_FILENAME_NEVER_USED) {
            break; // No more entries
        }

        // Compare with the given path
        if (strncmp((char *)entry, path, DIR_NAME_LEN) == 0) {
            // Check if entry is a directory
            if (entry[DIR_ATTRIBUTES_LEN] & ATT_DIRECTORY) {
                return 1; // Directory exists
            }
        }
    }

    return 0; // Directory does not exist
}

/**
 * Get next cluster
 * @param current_cluster Current cluster number
 * @return Next cluster number, -1 on failure
 */
int fat_driver_get_next_cluster(uint16_t current_cluster) {
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
        if (current_cluster % 2 == 0) {
            entry_value &= FAT_DRIVER_12_LAST_CLUSTER_END;
        } else {
            entry_value = (entry_value >> 4) & FAT_DRIVER_12_LAST_CLUSTER_END;
        }

        if (entry_value >= FAT_DRIVER_12_LAST_CLUSTER_START) return -1; // EOF
        if (entry_value == FAT_DRIVER_12_BAD_CLUSTER) {
            printf("Damaged cluster: %d\n", current_cluster);
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
        if (entry_value >= 0xFFF8) return -1; // EOF
        if (entry_value == 0xFFF7) {
            printf("Damaged cluster: %d\n", current_cluster);
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
        entry_value &= 0x0FFFFFFF; // Only take the lower 28 bits
        if (entry_value >= 0x0FFFFFF8) return -1; // EOF
        if (entry_value == 0x0FFFFFF7) {
            printf("Damaged cluster: %d\n", current_cluster);
            return -1;
        }
        return entry_value;
    }

    return -1; // Unsupported FAT type
}

// Hàm kiểm tra xem entry có bắt đầu bằng token và sau token là ranh giới (không phải ký tự chữ số hoặc chữ cái)
static int starts_with_token(const char *entry, const char *token) {
    size_t len = strlen(token);
    // So sánh đầu chuỗi
    if (strncmp(entry, token, len) != 0)
        return 0;
    // Kiểm tra ký tự sau token: nếu là '\0', dấu cách, hoặc ký tự không phải chữ hay số (ví dụ: dấu chấm, dấu phẩy)
    char next = entry[len];
    if (next == '\0' || isspace(next) || ispunct(next))
        return 1;
    return 0;
}

static int32_t reverseMax4Byte(const uint8_t *byte, uint32_t count)
{
    int32_t result = 0;
    if (count <= 4)
    {
        for (int i = count - 1; i >= 0; i--)
        {
            result = (result << 8) | byte[i];
        }
    }
    return result;
}
