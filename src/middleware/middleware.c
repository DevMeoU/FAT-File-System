/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Module Middleware cung cấp các hàm trung gian để khởi tạo hệ thống,
 *   liệt kê thư mục, thay đổi thư mục và đọc file dựa trên FAT Driver.
 *********************************************************************/

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdio.h>
#include <string.h>
#include "../utilities/linkedlist/linkedlist.h"
#include "middleware.h"
#include "../fat_driver/fat_driver.h"
#include "../utilities/log/print_color.h"

/*********************************************************************
 * Private Variables
 *********************************************************************/
/* List of Manager Node */
linkedlist_t manager_node_list;

/* Information Of Manager Node */
mid_node_info_t manager_node_info;

/* Biến toàn cục */
static MidControl mid_ctrl;

/*********************************************************************
 * Public Function Implementations
 *********************************************************************/

int32_t mid_init(const char* img_path)
{
    /* Khởi tạo FAT driver */
    if (fat_driver_init(img_path) != STATUS_SUCCESS) {
        print_error("Failed to initialize FAT driver\n");
        return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}

int32_t mid_init_with_file(const char *img_file_path, const char *env_path)
{
    /* Khởi tạo FAT driver */
    if (fat_driver_init(img_file_path) != STATUS_SUCCESS) {
        print_error("Failed to initialize FAT driver\n");
        return STATUS_ERROR;
    }

    /* Lưu đường dẫn environment */
    manager_node_info.env_path = strdup(env_path);
    if (manager_node_info.env_path == NULL) {
        print_error("Failed to allocate memory for environment path\n");
        return STATUS_NO_MEMORY;
    }

    return STATUS_SUCCESS;
}

int32_t mid_deinit(void)
{
    /* Giải phóng FAT driver */
    if (fat_driver_deinit() != STATUS_SUCCESS) {
        print_error("Failed to deinitialize FAT driver\n");
        return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}

int32_t mid_process_sync(const mid_cmd_t *cmd, mid_resp_t *resp)
{
    if (cmd == NULL || resp == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    /* Xử lý lệnh */
    switch (cmd->type) {
        case MID_CMD_READ:
            return mid_read_file((const char *)cmd->data, resp->data, cmd->size, &resp->size);
        
        case MID_CMD_WRITE:
            return mid_write_file((const char *)cmd->data, cmd->data + strlen((const char *)cmd->data) + 1,
                                cmd->size - strlen((const char *)cmd->data) - 1, &resp->size);
        
        case MID_CMD_SEND:
            return mid_send_data(cmd->data, cmd->size, cmd->timeout);
        
        case MID_CMD_RECEIVE:
            return mid_receive_data(resp->data, cmd->size, &resp->size, cmd->timeout);
        
        default:
            return STATUS_INVALID_PARAMETER;
    }
}

int32_t mid_process_async(const mid_cmd_t *cmd, mid_callback_t callback)
{
    if (cmd == NULL || callback == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    // TODO: Implement async processing
    return STATUS_ERROR;
}

int32_t mid_read_file(const char *path, void *buffer, uint32_t size, uint32_t *bytes_read)
{
    if (path == NULL || buffer == NULL || bytes_read == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    /* Mở file */
    fat_entry_t file;
    if (fat_driver_open(path, FAT_MODE_READ, &file) != STATUS_SUCCESS) {
        return STATUS_NOT_FOUND;
    }

    /* Đọc dữ liệu */
    int32_t status = fat_driver_read(&file, buffer, size, bytes_read);
    fat_driver_close(&file);

    return status;
}

int32_t mid_write_file(const char *path, const void *buffer, uint32_t size, uint32_t *bytes_written)
{
    if (path == NULL || buffer == NULL || bytes_written == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    /* Mở file */
    fat_entry_t file;
    if (fat_driver_open(path, FAT_MODE_WRITE | FAT_MODE_CREATE, &file) != STATUS_SUCCESS) {
        return STATUS_ERROR;
    }

    /* Ghi dữ liệu */
    int32_t status = fat_driver_write(&file, buffer, size, bytes_written);
    fat_driver_close(&file);

    return status;
}

int32_t mid_send_data(const void *data, uint32_t size, uint32_t timeout)
{
    if (data == NULL || size == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    (void)timeout; // Unused parameter
    // TODO: Implement data sending
    return STATUS_ERROR;
}

int32_t mid_receive_data(void *buffer, uint32_t size, uint32_t *bytes_received, uint32_t timeout)
{
    if (buffer == NULL || size == 0 || bytes_received == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    (void)timeout; // Unused parameter
    // TODO: Implement data receiving
    return STATUS_ERROR;
}

int32_t mid_init_mw(Middleware* mw, FATDriver* driver)
{
    if (!mw || !driver) {
        return STATUS_INVALID_PARAMETER;
    }

    mw->fat_driver = driver;
    strcpy(mw->current_path, "/");
    mw->current_dir_entries = NULL;
    mw->current_dir_count = 0;

    // Đọc thư mục gốc
    return mid_change_directory(mw, "/");
}

int32_t mid_list_directory(Middleware* mw)
{
    if (!mw) {
        return STATUS_INVALID_PARAMETER;
    }

    // In danh sách file/thư mục
    for (uint32_t i = 0; i < mw->current_dir_count; i++) {
        fat_entry_t* entry = &mw->current_dir_entries[i];
        printf("%s\t%s\t%u bytes\n",
               entry->attributes & FAT_ATTR_DIRECTORY ? "DIR" : "FILE",
               entry->name,
               entry->size);
    }

    return STATUS_SUCCESS;
}

int32_t mid_change_directory(Middleware* mw, const char* path)
{
    if (!mw || !path) {
        return STATUS_INVALID_PARAMETER;
    }

    char full_path[1024];
    int len = snprintf(full_path, sizeof(full_path), "%s/%s", mw->current_path, path);
    if (len >= sizeof(full_path)) {
        return STATUS_ERROR;
    }

    // Xử lý các trường hợp đặc biệt
    if (strcmp(path, "/") == 0) {
        // Chuyển về thư mục gốc
        strcpy(mw->current_path, "/");
        return fat_driver_read_root_dir(mw->fat_driver);
    }
    
    if (strcmp(path, ".") == 0) {
        // Giữ nguyên thư mục hiện tại
        return STATUS_SUCCESS;
    }

    if (strcmp(path, "..") == 0) {
        // Lên thư mục cha
        char* last_slash = strrchr(mw->current_path, '/');
        if (last_slash != mw->current_path) {
            *last_slash = '\0';
        }
        return mid_change_directory(mw, mw->current_path);
    }

    // Tìm và chuyển đến thư mục
    if (fat_driver_find_path_recursive(mw->fat_driver, full_path) == STATUS_SUCCESS) {
        strcpy(mw->current_path, full_path);
        return STATUS_SUCCESS;
    }

    return STATUS_ERROR;
}

int32_t mid_read_file_content(Middleware* mw, const char* filename, void* buffer, uint32_t* size)
{
    if (!mw || !filename || !buffer || !size) {
        return STATUS_INVALID_PARAMETER;
    }

    char full_path[1024];
    int len = snprintf(full_path, sizeof(full_path), "%s/%s", mw->current_path, filename);
    if (len >= sizeof(full_path)) {
        return STATUS_ERROR;
    }

    return fat_driver_read_file(mw->fat_driver, full_path, buffer, size);
}

void mid_cleanup(Middleware* mw)
{
    if (!mw) {
        return;
    }

    // Giải phóng bộ nhớ
    if (mw->current_dir_entries != NULL) {
        free(mw->current_dir_entries);
        mw->current_dir_entries = NULL;
    }

    // Reset các trường
    mw->fat_driver = NULL;
    mw->current_path[0] = '\0';
    mw->current_dir_count = 0;
}

/*********************************************************************
 * UUID: 8b9c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/

/* Hàm nội bộ */
static DirNode* create_node(void) {
    DirNode* node = malloc(sizeof(DirNode));
    if (node) {
        memset(node, 0, sizeof(DirNode));
    }
    return node;
}

static void free_node(DirNode* node) {
    if (node) {
        // Giải phóng các node con
        DirNode* child = node->children;
        while (child) {
            DirNode* next = child->next;
            free_node(child);
            child = next;
        }
        free(node);
    }
}

static DirNode* find_node(DirNode* root, const char* path) {
    if (!root || !path) return NULL;
    
    // Tách đường dẫn
    char path_copy[1024];
    strncpy(path_copy, path, sizeof(path_copy)-1);
    
    char* token = strtok(path_copy, "/");
    DirNode* current = root;
    
    while (token) {
        // Tìm trong danh sách con
        DirNode* found = NULL;
        DirNode* child = current->children;
        
        while (child) {
            if (strcmp(child->name, token) == 0) {
                found = child;
                break;
            }
            child = child->next;
        }
        
        if (!found) return NULL;
        current = found;
        token = strtok(NULL, "/");
    }
    
    return current;
}

static int build_tree(DirNode* node) {
    if (!node) return MID_ERROR;
    
    // Đọc thông tin entry
    FatEntry entry;
    if (fat_read_entry(node->first_cluster, &entry) != FAT_SUCCESS) {
        return MID_ERROR;
    }
    
    // Nếu là thư mục, đọc các entry con
    if (entry.attributes & FAT_ATTR_DIRECTORY) {
        DirNode* prev = NULL;
        uint32_t cluster = node->first_cluster;
        
        while (cluster && cluster < 0x0FFFFFF8) {
            // Đọc các entry trong cluster
            uint8_t buffer[512];
            uint32_t sector = ((cluster - 2) * entry.sectors_per_cluster) + entry.first_data_sector;
            
            if (fat_read_sector(sector, buffer) != FAT_SUCCESS) {
                return MID_ERROR;
            }
            
            // Duyệt qua các entry
            fat_dir_entry_t* dir_entry = (fat_dir_entry_t*)buffer;
            for (int i = 0; i < 16; i++, dir_entry++) {
                // Kiểm tra entry hợp lệ
                if (dir_entry->name[0] == 0) break;
                if (dir_entry->name[0] == 0xE5) continue;
                
                // Bỏ qua . và ..
                if (dir_entry->name[0] == '.') continue;
                
                // Tạo node mới
                DirNode* child = create_node();
                if (!child) return MID_ERROR;
                
                // Copy thông tin
                memcpy(child->name, dir_entry->name, 11);
                child->attributes = dir_entry->attr;
                child->size = dir_entry->file_size;
                child->first_cluster = (dir_entry->first_cluster_hi << 16) | dir_entry->first_cluster_lo;
                
                // Chuyển đổi thời gian
                uint16_t time = dir_entry->create_time;
                uint16_t date = dir_entry->create_date;
                
                child->create_time.hour = (time >> 11) & 0x1F;
                child->create_time.minute = (time >> 5) & 0x3F;
                child->create_time.second = (time & 0x1F) * 2;
                
                child->create_time.year = ((date >> 9) & 0x7F) + 1980;
                child->create_time.month = (date >> 5) & 0x0F;
                child->create_time.day = date & 0x1F;
                
                // Liên kết node
                child->parent = node;
                if (prev) {
                    prev->next = child;
                } else {
                    node->children = child;
                }
                prev = child;
                
                // Đệ quy xây dựng cây con nếu là thư mục
                if (child->attributes & FAT_ATTR_DIRECTORY) {
                    if (build_tree(child) != MID_SUCCESS) {
                        return MID_ERROR;
                    }
                }
            }
            
            // Đọc cluster tiếp theo
            cluster = get_next_cluster(cluster);
        }
    }
    
    return MID_SUCCESS;
}

/* Khởi tạo Middleware */
int mid_init(const char* img_path, const char* mode) {
    // Khởi tạo FAT Driver
    if (fat_init(img_path) != FAT_SUCCESS) {
        return MID_ERROR;
    }
    
    // Tạo node gốc
    mid_ctrl.root = create_node();
    if (!mid_ctrl.root) {
        mid_cleanup();
        return MID_ERROR;
    }
    
    // Khởi tạo thông tin node gốc
    strcpy(mid_ctrl.root->name, "/");
    mid_ctrl.root->attributes = FAT_ATTR_DIRECTORY;
    mid_ctrl.root->first_cluster = 2;  // Root directory cluster
    
    // Xây dựng cây thư mục
    if (build_tree(mid_ctrl.root) != MID_SUCCESS) {
        mid_cleanup();
        return MID_ERROR;
    }
    
    // Khởi tạo trạng thái
    mid_ctrl.current = mid_ctrl.root;
    strcpy(mid_ctrl.current_path, "/");
    mid_ctrl.is_root_mode = 1;
    
    return MID_SUCCESS;
}

/* Liệt kê nội dung thư mục */
int mid_list_dir(const char* path, DirNode** entries, int* count) {
    // Tìm node thư mục
    DirNode* dir = path ? find_node(mid_ctrl.root, path) : mid_ctrl.current;
    if (!dir || !(dir->attributes & FAT_ATTR_DIRECTORY)) {
        return MID_ERROR;
    }
    
    // Đếm số entry
    int n = 0;
    DirNode* child = dir->children;
    while (child) {
        n++;
        child = child->next;
    }
    
    // Cấp phát mảng entries
    *entries = malloc(sizeof(DirNode) * n);
    if (!*entries) {
        return MID_ERROR;
    }
    
    // Copy thông tin
    *count = n;
    n = 0;
    child = dir->children;
    while (child) {
        memcpy(&(*entries)[n++], child, sizeof(DirNode));
        child = child->next;
    }
    
    return MID_SUCCESS;
}

/* Thay đổi thư mục hiện tại */
int mid_change_dir(const char* path) {
    if (!path) return MID_ERROR;
    
    // Xử lý các trường hợp đặc biệt
    if (strcmp(path, "/") == 0) {
        mid_ctrl.current = mid_ctrl.root;
        strcpy(mid_ctrl.current_path, "/");
        mid_ctrl.is_root_mode = 1;
        return MID_SUCCESS;
    }
    
    if (strcmp(path, ".") == 0 || strcmp(path, "./") == 0) {
        return MID_SUCCESS;
    }
    
    if (strcmp(path, "..") == 0 || strcmp(path, "../") == 0) {
        if (mid_ctrl.current->parent) {
            mid_ctrl.current = mid_ctrl.current->parent;
            char* last_slash = strrchr(mid_ctrl.current_path, '/');
            if (last_slash != mid_ctrl.current_path) {
                *last_slash = '\0';
            } else {
                strcpy(mid_ctrl.current_path, "/");
            }
            mid_ctrl.is_root_mode = (mid_ctrl.current == mid_ctrl.root);
        }
        return MID_SUCCESS;
    }
    
    // Tìm node thư mục
    DirNode* dir = find_node(mid_ctrl.root, path);
    if (!dir || !(dir->attributes & FAT_ATTR_DIRECTORY)) {
        return MID_ERROR;
    }
    
    // Cập nhật trạng thái
    mid_ctrl.current = dir;
    if (path[0] == '/') {
        strncpy(mid_ctrl.current_path, path, sizeof(mid_ctrl.current_path)-1);
    } else {
        if (strcmp(mid_ctrl.current_path, "/") != 0) {
            strncat(mid_ctrl.current_path, "/", sizeof(mid_ctrl.current_path)-1);
        }
        strncat(mid_ctrl.current_path, path, sizeof(mid_ctrl.current_path)-1);
    }
    mid_ctrl.is_root_mode = (dir == mid_ctrl.root);
    
    return MID_SUCCESS;
}

/* Đọc nội dung file */
int mid_read_file(const char* path, void* buffer, uint32_t size) {
    // Tìm node file
    DirNode* file = find_node(mid_ctrl.root, path);
    if (!file || (file->attributes & FAT_ATTR_DIRECTORY)) {
        return MID_ERROR;
    }
    
    // Kiểm tra kích thước
    if (size > file->size) {
        size = file->size;
    }
    
    // Đọc nội dung file
    uint32_t cluster = file->first_cluster;
    uint32_t bytes_read = 0;
    uint8_t* buf = buffer;
    
    while (cluster && cluster < 0x0FFFFFF8 && bytes_read < size) {
        // Đọc cluster
        uint8_t cluster_buf[512];
        uint32_t sector = ((cluster - 2) * file->sectors_per_cluster) + file->first_data_sector;
        
        if (fat_read_sector(sector, cluster_buf) != FAT_SUCCESS) {
            return MID_ERROR;
        }
        
        // Copy dữ liệu
        uint32_t bytes_to_copy = size - bytes_read;
        if (bytes_to_copy > 512) bytes_to_copy = 512;
        
        memcpy(buf + bytes_read, cluster_buf, bytes_to_copy);
        bytes_read += bytes_to_copy;
        
        // Đọc cluster tiếp theo
        cluster = get_next_cluster(cluster);
    }
    
    return bytes_read;
}

/* Dọn dẹp */
void mid_cleanup(void) {
    // Giải phóng cây thư mục
    free_node(mid_ctrl.root);
    
    // Dọn dẹp FAT Driver
    fat_cleanup();
    
    memset(&mid_ctrl, 0, sizeof(MidControl));
}
