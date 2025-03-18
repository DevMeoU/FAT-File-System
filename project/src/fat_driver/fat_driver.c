#include "fat_driver.h"
#include "fat_driver_private.h"
#include <stdlib.h>
#include <string.h>

// Khai báo hàm static
static int fat_driver_load_fat_table(FATDriver* driver);
static int fat_driver_load_root_directory(FATDriver* driver);
static int fat_driver_build_directory_tree(FATDriver* driver);
static void fat_driver_parse_boot_sector(FATDriver* driver, const uint8_t* boot_sector_buffer);

int fat_driver_init(FATDriver* driver, const FileSystemConfig config) {
    // Khởi tạo HAL
    HAL* hal = malloc(sizeof(HAL));
    if (hal_init(hal, config.img_path, SECTOR_SIZE_512) != 0) {
        return -1;
    }
    if (!driver || !hal) return -1;

    
    // Khởi tạo các thành phần của driver
    memset(driver, 0, sizeof(FATDriver));
    driver->hal = hal;
    driver->config = config;
    
    // Cấp phát bộ nhớ cho cache
    driver->cache_size = (uint32_t)config.cache_size;
    driver->cache = malloc(driver->cache_size * hal_get_sector_size(hal));
    if (!driver->cache) return -1;
    
    return 0;
}

int fat_driver_deinit(FATDriver* driver) {
    if (!driver || !driver->hal) return -1;
    
    hal_deinit(driver->hal);
    free(driver->cache);
    return 0;
}

int fat_driver_mount(FATDriver* driver) {
    if (!driver || !driver->hal) return -1;
    
    uint8_t* boot_sector_buffer = malloc(hal_get_sector_size(driver->hal));
    if (!boot_sector_buffer) return -1;
    
    // Đọc boot sector
    uint32_t bytes_read = hal_read_sector(driver->hal, 0, boot_sector_buffer);
    if (bytes_read != (uint32_t)hal_get_sector_size(driver->hal)) {
        free(boot_sector_buffer);
        return -1;
    }
    
    // Phân tích boot sector
    fat_driver_parse_boot_sector(driver, boot_sector_buffer);
    free(boot_sector_buffer);
    
    // Tính toán các thông số cần thiết
    driver->first_fat_sector = driver->boot_sector.reserved_sectors;
    
    // Tính số sector của thư mục gốc (chỉ áp dụng cho FAT12/16)
    driver->root_dir_sectors = ((driver->boot_sector.root_entry_count * 32) + 
                               (driver->boot_sector.bytes_per_sector - 1)) / 
                               driver->boot_sector.bytes_per_sector;
    
    // Tính sector đầu tiên của thư mục gốc
    driver->first_root_dir_sector = driver->boot_sector.reserved_sectors + 
                                   (driver->boot_sector.number_of_fats * 
                                   (driver->boot_sector.fat_size_16 ? 
                                    driver->boot_sector.fat_size_16 : 
                                    driver->boot_sector.fat_size_32));
    
    // Tính sector đầu tiên của vùng dữ liệu
    if (fat_driver_get_fat_type(driver) == FAT_TYPE_32) {
        driver->first_data_sector = driver->boot_sector.reserved_sectors + 
                                   (driver->boot_sector.number_of_fats * 
                                    driver->boot_sector.fat_size_32);
    } else {
        driver->first_data_sector = driver->first_root_dir_sector + 
                                   driver->root_dir_sectors;
    }
    
    // Tính tổng số sector dữ liệu
    uint32_t total_sectors = driver->boot_sector.total_sectors_16 ? 
                            driver->boot_sector.total_sectors_16 : 
                            driver->boot_sector.total_sectors_32;
    
    driver->data_sectors = total_sectors - 
                          (driver->boot_sector.reserved_sectors + 
                          (driver->boot_sector.number_of_fats * 
                          (driver->boot_sector.fat_size_16 ? 
                           driver->boot_sector.fat_size_16 : 
                           driver->boot_sector.fat_size_32)) + 
                          driver->root_dir_sectors);
    
    // Tính tổng số cluster
    driver->total_clusters = driver->data_sectors / 
                            driver->boot_sector.sectors_per_cluster;
    
    // Load bảng FAT
    if (fat_driver_load_fat_table(driver) != 0) {
        return -1;
    }
    
    // Load thư mục gốc
    if (fat_driver_load_root_directory(driver) != 0) {
        return -1;
    }
    
    // Xây dựng cây thư mục
    if (fat_driver_build_directory_tree(driver) != 0) {
        return -1;
    }
    
    // Đặt thư mục hiện tại là thư mục gốc
    driver->current_directory = driver->root_directory;
    
    return 0;
}

void fat_driver_unmount(FATDriver* driver) {
    if (!driver) return;
    
    // Giải phóng bộ nhớ
    if (driver->fat_table) {
        free(driver->fat_table);
        driver->fat_table = NULL;
    }
    
    if (driver->cache) {
        free(driver->cache);
        driver->cache = NULL;
    }
    
    // Giải phóng cây thư mục
    if (driver->root_directory) {
        fat_driver_free_file_node(driver->root_directory);
        driver->root_directory = NULL;
    }
    
    driver->current_directory = NULL;
}

FileNode* fat_driver_get_root_directory(FATDriver* driver) {
    if (!driver) return NULL;
    
    return driver->root_directory;
}

FileNode* fat_driver_get_current_directory(FATDriver* driver) {
    if (!driver) return NULL;
    
    return driver->current_directory;
}

int fat_driver_set_current_directory(FATDriver* driver, FileNode* directory) {
    if (!driver || !directory || directory->type != FILE_TYPE_DIRECTORY) {
        return -1;
    }
    
    driver->current_directory = directory;
    return 0;
}

FileNode* fat_driver_find_path(FATDriver* driver, const char* path) {
    if (!driver || !path) return NULL;
    
    // Xử lý đường dẫn tuyệt đối
    if (path[0] == '/') {
        if (path[1] == '\0') {
            return driver->root_directory;
        }
        
        // Bỏ qua ký tự '/' đầu tiên
        path++;
        FileNode* current = driver->root_directory;
        return fat_driver_find_path_recursive(current, path);
    }
    
    // Xử lý đường dẫn tương đối
    FileNode* current = driver->current_directory;
    return fat_driver_find_path_recursive(current, path);
}

// Hàm đệ quy để tìm đường dẫn
FileNode* fat_driver_find_path_recursive(FileNode* current, const char* path) {
    if (!current || !path || path[0] == '\0') {
        return current;
    }
    
    // Tách phần đầu tiên của đường dẫn
    char component[FILE_NAME_MAX + 1];
    const char* next_path = NULL;
    
    const char* slash = strchr(path, '/');
    if (slash) {
        size_t len = slash - path;
        if (len > FILE_NAME_MAX) len = FILE_NAME_MAX;
        strncpy(component, path, len);
        component[len] = '\0';
        next_path = slash + 1;
    } else {
        strncpy(component, path, FILE_NAME_MAX);
        component[FILE_NAME_MAX] = '\0';
        next_path = path + strlen(path);
    }
    
    // Xử lý các trường hợp đặc biệt
    if (strcmp(component, ".") == 0) {
        return fat_driver_find_path_recursive(current, next_path);
    } else if (strcmp(component, "..") == 0) {
        if (current->parent) {
            return fat_driver_find_path_recursive(current->parent, next_path);
        } else {
            return fat_driver_find_path_recursive(current, next_path);
        }
    }
    
    // Tìm trong danh sách con
    FileNode* child = current->children;
    while (child) {
        if (strcmp(child->name, component) == 0) {
            if (*next_path == '\0') {
                return child;
            } else if (child->type == FILE_TYPE_DIRECTORY) {
                return fat_driver_find_path_recursive(child, next_path);
            } else {
                return NULL; // Không thể điều hướng vào file
            }
        }
        child = child->next;
    }
    
    return NULL; // Không tìm thấy
}

int fat_driver_read_file(FATDriver* driver, FileNode* file, void* buffer, uint32_t size) {
    if (!driver || !file || !buffer || file->type != FILE_TYPE_REGULAR) {
        return -1;
    }
    
    // Kiểm tra mode
    if (driver->config.mode == MODE_READ_ONLY || driver->config.mode == MODE_READ_WRITE) {
        uint32_t bytes_to_read = size;
        if (bytes_to_read > file->size) {
            bytes_to_read = file->size;
        }
        
        uint32_t bytes_read = 0;
        uint32_t current_cluster = file->first_cluster;
        uint32_t sector_size = hal_get_sector_size(driver->hal);
        uint32_t sectors_per_cluster = driver->boot_sector.sectors_per_cluster;
        uint8_t* temp_buffer = malloc(sector_size);
        
        if (!temp_buffer) return -1;
        
        while (bytes_read < bytes_to_read && current_cluster != 0 && 
               current_cluster != FAT12_EOC && 
               current_cluster != FAT16_EOC && 
               current_cluster != FAT32_EOC) {
            
            uint32_t first_sector_of_cluster = fat_driver_cluster_to_sector(driver, current_cluster);
            
            for (uint32_t i = 0; i < sectors_per_cluster && bytes_read < bytes_to_read; i++) {
                uint32_t read_bytes = hal_read_sector(driver->hal, first_sector_of_cluster + i, temp_buffer);
                if (read_bytes != sector_size) {
                    free(temp_buffer);
                    return -1;
                }
                
                uint32_t bytes_to_copy = bytes_to_read - bytes_read;
                if (bytes_to_copy > sector_size) {
                    bytes_to_copy = sector_size;
                }
                
                memcpy((uint8_t*)buffer + bytes_read, temp_buffer, bytes_to_copy);
                bytes_read += bytes_to_copy;
            }
            
            // Lấy cluster tiếp theo
            current_cluster = fat_driver_get_next_cluster(driver, current_cluster);
        }
        
        free(temp_buffer);
        return bytes_read;
    }
    
    return -1;
}

int fat_driver_write_file(FATDriver* driver, FileNode* file, const void* buffer, uint32_t size) {
    if (!driver || !file || !buffer || file->type != FILE_TYPE_REGULAR) {
        return -1;
    }
    
    // Kiểm tra mode
    if (driver->config.mode == MODE_READ_WRITE) {
        // Triển khai ghi file
        // ...
        
        return size;
    }
    
    return -1;
}

FatType fat_driver_get_fat_type(FATDriver* driver) {
    if (!driver) return FAT_TYPE_UNKNOWN;
    
    uint32_t total_clusters = driver->total_clusters;
    
    if (total_clusters < 4085) {
        return FAT_TYPE_12;
    } else if (total_clusters < 65525) {
        return FAT_TYPE_16;
    } else {
        return FAT_TYPE_32;
    }
}

int fat_driver_get_filesystem_info(FATDriver* driver, uint64_t* total_size, uint64_t* free_size) {
    if (!driver || !total_size || !free_size) return -1;
    
    uint32_t cluster_size = driver->boot_sector.sectors_per_cluster * 
                           driver->boot_sector.bytes_per_sector;
    
    *total_size = (uint64_t)driver->total_clusters * cluster_size;
    
    // Đếm số cluster trống
    uint32_t free_clusters = 0;
    for (uint32_t i = 2; i < driver->total_clusters + 2; i++) {
        if (fat_driver_get_fat_entry(driver, i) == 0) {
            free_clusters++;
        }
    }
    
    *free_size = (uint64_t)free_clusters * cluster_size;
    
    return 0;
}

uint32_t fat_driver_cluster_to_sector(FATDriver* driver, uint32_t cluster) {
    if (!driver || cluster < 2) return 0;
    
    return driver->first_data_sector + 
          (cluster - 2) * driver->boot_sector.sectors_per_cluster;
}

void fat_driver_free_file_node(FileNode* node) {
    if (!node) return;
    
    // Giải phóng các node con trước
    FileNode* child = node->children;
    while (child) {
        FileNode* next = child->next;
        fat_driver_free_file_node(child);
        child = next;
    }
    
    // Giải phóng node hiện tại
    free(node);
}

// Hàm nội bộ để phân tích boot sector
static void fat_driver_parse_boot_sector(FATDriver* driver, const uint8_t* boot_sector_buffer) {
    if (!driver || !boot_sector_buffer) return;
    
    BootSector* bs = &driver->boot_sector;
    
    // Sao chép các trường từ boot sector
    bs->bytes_per_sector = *(uint16_t*)(boot_sector_buffer + 11);
    bs->sectors_per_cluster = *(uint8_t*)(boot_sector_buffer + 13);
    bs->reserved_sectors = *(uint16_t*)(boot_sector_buffer + 14);
    bs->number_of_fats = *(uint8_t*)(boot_sector_buffer + 16);
    bs->root_entry_count = *(uint16_t*)(boot_sector_buffer + 17);
    bs->total_sectors_16 = *(uint16_t*)(boot_sector_buffer + 19);
    bs->media_type = *(uint8_t*)(boot_sector_buffer + 21);
    bs->fat_size_16 = *(uint16_t*)(boot_sector_buffer + 22);
    bs->sectors_per_track = *(uint16_t*)(boot_sector_buffer + 24);
    bs->number_of_heads = *(uint16_t*)(boot_sector_buffer + 26);
    bs->hidden_sectors = *(uint32_t*)(boot_sector_buffer + 28);
    bs->total_sectors_32 = *(uint32_t*)(boot_sector_buffer + 32);
    
    // Kiểm tra xem có phải FAT32 không
    if (bs->fat_size_16 == 0) {
        bs->fat_size_32 = *(uint32_t*)(boot_sector_buffer + 36);
        bs->extended_flags = *(uint16_t*)(boot_sector_buffer + 40);
        bs->fs_version = *(uint16_t*)(boot_sector_buffer + 42);
        bs->root_cluster = *(uint32_t*)(boot_sector_buffer + 44);
        bs->fs_info = *(uint16_t*)(boot_sector_buffer + 48);
        bs->backup_boot_sector = *(uint16_t*)(boot_sector_buffer + 50);
        // Sao chép 12 byte dành riêng
        memcpy(bs->reserved, boot_sector_buffer + 52, 12);
    }
    
    // Sao chép các trường chung
    bs->drive_number = *(uint8_t*)(boot_sector_buffer + (bs->fat_size_16 == 0 ? 64 : 36));
    bs->reserved1 = *(uint8_t*)(boot_sector_buffer + (bs->fat_size_16 == 0 ? 65 : 37));
    bs->boot_signature = *(uint8_t*)(boot_sector_buffer + (bs->fat_size_16 == 0 ? 66 : 38));
    bs->volume_id = *(uint32_t*)(boot_sector_buffer + (bs->fat_size_16 == 0 ? 67 : 39));
    
    // Sao chép volume label (11 byte)
    memcpy(bs->volume_label, boot_sector_buffer + (bs->fat_size_16 == 0 ? 71 : 43), 11);
    // bs->volume_label[11] = '\0';
    
    // Sao chép file system type (8 byte)
    memcpy(bs->fs_type, boot_sector_buffer + (bs->fat_size_16 == 0 ? 82 : 54), 8);
    // bs->fs_type[8] = '\0';
}

// Hàm nội bộ để load bảng FAT
static int fat_driver_load_fat_table(FATDriver* driver) {
    if (!driver) return -1;
    
    uint32_t fat_size;
    if (driver->boot_sector.fat_size_16 != 0) {
        fat_size = driver->boot_sector.fat_size_16;
    } else {
        fat_size = driver->boot_sector.fat_size_32;
    }
    
    uint32_t fat_size_bytes = fat_size * driver->boot_sector.bytes_per_sector;
    driver->fat_table = malloc(fat_size_bytes);
    if (!driver->fat_table) return -1;
    
    uint32_t sector_size = hal_get_sector_size(driver->hal);
    uint8_t* buffer = malloc(sector_size);
    if (!buffer) {
        free(driver->fat_table);
        driver->fat_table = NULL;
        return -1;
    }
    
    for (uint32_t i = 0; i < fat_size; i++) {
        uint32_t read_bytes = hal_read_sector(driver->hal, driver->first_fat_sector + i, buffer);
        if (read_bytes != sector_size) {
            free(buffer);
            free(driver->fat_table);
            driver->fat_table = NULL;
            return -1;
        }
        
        memcpy((uint8_t*)driver->fat_table + i * sector_size, buffer, sector_size);
    }
    
    free(buffer);
    return 0;
}

// Hàm nội bộ để load thư mục gốc
static int fat_driver_load_root_directory(FATDriver* driver) {
    if (!driver) return -1;
    
    // Tạo node cho thư mục gốc
    driver->root_directory = malloc(sizeof(FileNode));
    if (!driver->root_directory) return -1;
    
    memset(driver->root_directory, 0, sizeof(FileNode));
    strcpy(driver->root_directory->name, "/");
    driver->root_directory->type = FILE_TYPE_DIRECTORY;
    driver->root_directory->attributes.directory = true; // FAT_ATTR_DIRECTORY;
    
    if (fat_driver_get_fat_type(driver) == FAT_TYPE_32) {
        driver->root_directory->first_cluster = driver->boot_sector.root_cluster;
    } else {
        driver->root_directory->first_cluster = 0; // Thư mục gốc trong FAT12/16 không nằm trong vùng dữ liệu
    }
    
    return 0;
}

// Hàm nội bộ để xây dựng cây thư mục
static int fat_driver_build_directory_tree(FATDriver* driver) {
    if (!driver || !driver->root_directory) return -1;
    
    uint32_t sector_size = hal_get_sector_size(driver->hal);
    uint8_t* buffer = malloc(sector_size);
    if (!buffer) return -1;
    
    // Xử lý thư mục gốc
    if (fat_driver_get_fat_type(driver) == FAT_TYPE_32) {
        // Trong FAT32, thư mục gốc là một cluster chain
        uint32_t current_cluster = driver->root_directory->first_cluster;
        
        while (current_cluster != 0 && 
               current_cluster != FAT32_EOC) {
            
            uint32_t first_sector_of_cluster = fat_driver_cluster_to_sector(driver, current_cluster);
            
            for (uint32_t i = 0; i < driver->boot_sector.sectors_per_cluster; i++) {
                uint32_t read_bytes = hal_read_sector(driver->hal, first_sector_of_cluster + i, buffer);
                if (read_bytes != sector_size) {
                    free(buffer);
                    return -1;
                }
                
                // Xử lý các entry trong sector
                for (uint32_t j = 0; j < sector_size; j += 32) {
                    FATDirEntry* entry = (FATDirEntry*)(buffer + j);
                    
                    // Kiểm tra entry trống hoặc đã xóa
                    if (entry->name[0] == 0x00 || entry->name[0] == (uint8_t)0xE5) {
                        continue;
                    }
                    
                    // Bỏ qua entry volume label
                    if (entry->attributes & FAT_ATTR_VOLUME_ID) {
                        continue;
                    }
                    
                    // Tạo node mới
                    FileNode* node = malloc(sizeof(FileNode));
                    if (!node) {
                        free(buffer);
                        return -1;
                    }
                    
                    // Điền thông tin cho node
                    fat_driver_fill_file_node(driver, node, entry);
                    
                    // Thêm node vào thư mục gốc
                    node->parent = driver->root_directory;
                    node->next = driver->root_directory->children;
                    driver->root_directory->children = node;
                }
            }
            
            // Lấy cluster tiếp theo
            current_cluster = fat_driver_get_next_cluster(driver, current_cluster);
        }
    } else {
        // Trong FAT12/16, thư mục gốc nằm ở vị trí cố định
        for (uint32_t i = 0; i < driver->root_dir_sectors; i++) {
            uint32_t read_bytes = hal_read_sector(driver->hal, driver->first_root_dir_sector + i, buffer);
            if (read_bytes != sector_size) {
                free(buffer);
                return -1;
            }
            
            // Xử lý các entry trong sector
            for (uint32_t j = 0; j < sector_size; j += 32) {
                FATDirEntry* entry = (FATDirEntry*)(buffer + j);
                
                // Kiểm tra entry trống hoặc đã xóa
                if (entry->name[0] == 0x00 || entry->name[0] == (uint8_t)0xE5) {
                    continue;
                }
                
                // Bỏ qua entry volume label
                if (entry->attributes & FAT_ATTR_VOLUME_ID) {
                    continue;
                }
                
                // Tạo node mới
                FileNode* node = malloc(sizeof(FileNode));
                if (!node) {
                    free(buffer);
                    return -1;
                }
                
                // Điền thông tin cho node
                fat_driver_fill_file_node(driver, node, entry);
                
                // Thêm node vào thư mục gốc
                node->parent = driver->root_directory;
                node->next = driver->root_directory->children;
                driver->root_directory->children = node;
            }
        }
    }
    
    // Xử lý các thư mục con
    FileNode* current = driver->root_directory->children;
    while (current) {
        if (current->type == FILE_TYPE_DIRECTORY) {
            fat_driver_build_directory_tree_recursive(driver, current);
        }
        current = current->next;
    }
    
    free(buffer);
    return 0;
}

// Hàm đệ quy để xây dựng cây thư mục
int fat_driver_build_directory_tree_recursive(FATDriver* driver, FileNode* directory) {
    if (!driver || !directory || directory->type != FILE_TYPE_DIRECTORY) {
        return -1;
    }
    
    uint32_t sector_size = hal_get_sector_size(driver->hal);
    uint8_t* buffer = malloc(sector_size);
    if (!buffer) return -1;
    
    uint32_t current_cluster = directory->first_cluster;
    
    while (current_cluster != 0 && 
           current_cluster != FAT12_EOC && 
           current_cluster != FAT16_EOC && 
           current_cluster != FAT32_EOC) {
        
        uint32_t first_sector_of_cluster = fat_driver_cluster_to_sector(driver, current_cluster);
        
        for (uint32_t i = 0; i < driver->boot_sector.sectors_per_cluster; i++) {
            uint32_t read_bytes = hal_read_sector(driver->hal, first_sector_of_cluster + i, buffer);
            if (read_bytes != sector_size) {
                free(buffer);
                return -1;
            }
            
            // Xử lý các entry trong sector
            for (uint32_t j = 0; j < sector_size; j += 32) {
                FATDirEntry* entry = (FATDirEntry*)(buffer + j);
                
                // Kiểm tra entry trống hoặc đã xóa
                if (entry->name[0] == 0x00 || entry->name[0] == (uint8_t)0xE5) {
                    continue;
                }
                
                // Bỏ qua entry volume label và entry . và ..
                if ((entry->attributes & FAT_ATTR_VOLUME_ID) ||
                    (entry->name[0] == '.' && entry->name[1] == ' ') ||
                    (entry->name[0] == '.' && entry->name[1] == '.' && entry->name[2] == ' ')) {
                    continue;
                }
                
                // Tạo node mới
                FileNode* node = malloc(sizeof(FileNode));
                if (!node) {
                    free(buffer);
                    return -1;
                }
                
                // Điền thông tin cho node
                fat_driver_fill_file_node(driver, node, entry);
                
                // Thêm node vào thư mục hiện tại
                node->parent = directory;
                node->next = directory->children;
                directory->children = node;
            }
        }
        
        // Lấy cluster tiếp theo
        current_cluster = fat_driver_get_next_cluster(driver, current_cluster);
    }
    
    // Xử lý các thư mục con
    FileNode* current = directory->children;
    while (current) {
        if (current->type == FILE_TYPE_DIRECTORY) {
            fat_driver_build_directory_tree_recursive(driver, current);
        }
        current = current->next;
    }
    
    free(buffer);
    return 0;
}

// Hàm để điền thông tin cho node từ entry
void fat_driver_fill_file_node(FATDriver* driver, FileNode* node, const FATDirEntry* entry) {
    if (!driver || !node || !entry) return;
    
    memset(node, 0, sizeof(FileNode));
    
    // Chuyển đổi tên file từ định dạng 8.3
    char name[13] = {0};
    int name_len = 0;
    
    // Xử lý phần tên (8 ký tự)
    for (int i = 0; i < 8; i++) {
        if (entry->name[i] != ' ') {
            name[name_len++] = entry->name[i];
        }
    }
    
    // Xử lý phần mở rộng (3 ký tự)
    if (entry->ext[0] != ' ') {
        name[name_len++] = '.';
        for (int i = 0; i < 3; i++) {
            if (entry->ext[i] != ' ') {
                name[name_len++] = entry->ext[i];
            }
        }
    }
    
    name[name_len] = '\0';
    
    // Chuyển tên thành chữ thường
    for (int i = 0; i < name_len; i++) {
        if (name[i] >= 'A' && name[i] <= 'Z') {
            name[i] = name[i] - 'A' + 'a';
        }
    }
    
    strcpy(node->name, name);
    
    // Điền các thông tin khác
    node->size = entry->file_size;
    node->attributes.directory = true; // FAT_ATTR_DIRECTORY;
    
    if (entry->attributes & FAT_ATTR_DIRECTORY) {
        node->type = FILE_TYPE_DIRECTORY;
    } else {
        node->type = FILE_TYPE_REGULAR;
    }
    
    // Tính cluster đầu tiên
    node->first_cluster = entry->first_cluster_low;
    if (fat_driver_get_fat_type(driver) == FAT_TYPE_32) {
        node->first_cluster |= ((uint32_t)entry->first_cluster_high << 16);
    }
    
    // Điền thông tin thời gian
    node->created_time.year = 1980 + ((entry->create_date >> 9) & 0x7F);
    node->created_time.month = (entry->create_date >> 5) & 0x0F;
    node->created_time.day = entry->create_date & 0x1F;
    node->created_time.hour = (entry->create_time >> 11) & 0x1F;
    node->created_time.minute = (entry->create_time >> 5) & 0x3F;
    node->created_time.second = (entry->create_time & 0x1F) * 2;
    
    node->modified_time.year = 1980 + ((entry->write_date >> 9) & 0x7F);
    node->modified_time.month = (entry->write_date >> 5) & 0x0F;
    node->modified_time.day = entry->write_date & 0x1F;
    node->modified_time.hour = (entry->write_time >> 11) & 0x1F;
    node->modified_time.minute = (entry->write_time >> 5) & 0x3F;
    node->modified_time.second = (entry->write_time & 0x1F) * 2;
}

// Hàm để lấy giá trị entry trong bảng FAT
uint32_t fat_driver_get_fat_entry(FATDriver* driver, uint32_t cluster) {
    if (!driver || !driver->fat_table) return 0;
    
    FatType fat_type = fat_driver_get_fat_type(driver);
    
    if (fat_type == FAT_TYPE_12) {
        uint32_t fat_offset = cluster + (cluster / 2);
        uint16_t fat_entry = *(uint16_t*)((uint8_t*)driver->fat_table + fat_offset);
        
        if (cluster & 0x1) {
            return fat_entry >> 4; // Cluster lẻ
        } else {
            return fat_entry & 0x0FFF; // Cluster chẵn
        }
    } else if (fat_type == FAT_TYPE_16) {
        uint32_t fat_offset = cluster * 2;
        return *(uint16_t*)((uint8_t*)driver->fat_table + fat_offset);
    } else if (fat_type == FAT_TYPE_32) {
        uint32_t fat_offset = cluster * 4;
        return *(uint32_t*)((uint8_t*)driver->fat_table + fat_offset) & 0x0FFFFFFF;
    }
    
    return 0;
}

// Hàm để lấy cluster tiếp theo trong chuỗi cluster
uint32_t fat_driver_get_next_cluster(FATDriver* driver, uint32_t cluster) {
    if (!driver) return 0;
    
    return fat_driver_get_fat_entry(driver, cluster);
}
