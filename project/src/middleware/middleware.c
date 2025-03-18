#include "middleware.h"
#include "../utilities/log/print_color.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int middleware_init(Middleware* middleware) {
    FATDriver* fat_driver = malloc(sizeof(FATDriver)); /* Khai báo cấu trúc Middleware */
    if (!middleware || !fat_driver) return -1;
    
    // Kiểm tra tham số đầu vào
    if (!middleware->img_path) {
        print_error("Image file path is required\n");
        return -1;
    }
    
    const char* ext = strrchr(middleware->img_path, '.');
    if (!ext || strcmp(ext, ".img") != 0) {
        print_error("File is not an image file (.img): %s\n", middleware->img_path);
        return -1;
    }
    
    // Cấu hình hệ thống tệp
    FileSystemConfig config;
    config.img_path = middleware->img_path;
    config.mode = middleware->mode;
    config.fat_type = FAT_TYPE_16; // Mặc định, sẽ được xác định lại trong fat_driver_mount
    config.sector_size = SECTOR_SIZE_512;
    config.cache_size = CACHE_SIZE_16;
    config.dir_name_len = DIR_NAME_LEN_8;
    
    middleware->fat_driver = fat_driver;
    if (fat_driver_init(fat_driver, config) != 0) {
        print_error("Failed to initialize FAT Driver\n");
        fat_driver_deinit(fat_driver);
        return -1;
    }
    
    // Mount hệ thống tệp
    if (fat_driver_mount(fat_driver) != 0) {
        print_error("Failed to mount file system\n");
        fat_driver_unmount(fat_driver);
        fat_driver_deinit(fat_driver);
        return -1;
    }
    
    print_success("Mount successful\n");
    
    // Chuyển chế độ dựa trên đường dẫn
    if (strcmp(middleware->img_path, "/") == 0) {
        middleware_switch_to_root_mode(middleware);
    } else {
        middleware_switch_to_user_mode(middleware);
    }
    
    middleware->current_directory = fat_driver_get_root_directory(fat_driver);
    strcpy(middleware->current_path, "/");
    middleware->is_root_mode = true;

    return 0;
}

int middleware_denit(Middleware* middleware) {
    if (!middleware) return -1;
    
    if (middleware->fat_driver) {
        fat_driver_unmount(middleware->fat_driver);
        fat_driver_deinit(middleware->fat_driver);
        free(middleware->fat_driver);
    }
    
    return 0;
}

int middleware_ls(Middleware* middleware) {
    if (!middleware || !middleware->current_directory) return -1;
    
    FileNode* current = middleware->current_directory->children;
    
    if (!current) {
        printf("Directory is empty\n");
        return 0;
    }
    
    printf("%-32s %-12s %-12s %-20s %-20s\n", 
           "Name", "Type", "Size", "Created", "Modified");
    printf("--------------------------------------------------------------------------------\n");
    
    while (current) {
        char type_str[16] = {0};
        if (current->type == FILE_TYPE_DIRECTORY) {
            strcpy(type_str, "Directory");
        } else if (current->type == FILE_TYPE_REGULAR) {
            strcpy(type_str, "File");
        } else if (current->type == FILE_TYPE_VOLUME_ID) {
            strcpy(type_str, "Volume ID");
        } else {
            strcpy(type_str, "Unknown");
        }
        
        char created[32] = {0};
        char modified[32] = {0};
        
        if (current->created_time.year > 0) {
            sprintf(created, "%04d-%02d-%02d %02d:%02d:%02d", 
                    current->created_time.year, 
                    current->created_time.month, 
                    current->created_time.day,
                    current->created_time.hour,
                    current->created_time.minute,
                    current->created_time.second);
        } else {
            strcpy(created, "N/A");
        }
        
        if (current->modified_time.year > 0) {
            sprintf(modified, "%04d-%02d-%02d %02d:%02d:%02d", 
                    current->modified_time.year, 
                    current->modified_time.month, 
                    current->modified_time.day,
                    current->modified_time.hour,
                    current->modified_time.minute,
                    current->modified_time.second);
        } else {
            strcpy(modified, "N/A");
        }
        
        printf("%-32s %-12s %-12u %-20s %-20s\n", 
               current->name, 
               type_str, 
               current->size, 
               created, 
               modified);
        
        current = current->next;
    }
    
    return 0;
}

int middleware_cd(Middleware* middleware, const char* path) {
    if (!middleware || !path) return -1;
    
    // Xử lý các trường hợp đặc biệt
    if (strcmp(path, "/") == 0) {
        // Chuyển về thư mục gốc
        middleware->current_directory = fat_driver_get_root_directory(middleware->fat_driver);
        strcpy(middleware->current_path, "/");
        return 0;
    } else if (strcmp(path, ".") == 0 || strcmp(path, "./") == 0) {
        // Giữ nguyên thư mục hiện tại
        return 0;
    } else if (strcmp(path, "..") == 0 || strcmp(path, "../") == 0) {
        // Chuyển về thư mục cha
        if (middleware->current_directory->parent) {
            middleware->current_directory = middleware->current_directory->parent;
            
            // Cập nhật đường dẫn hiện tại
            char* last_slash = strrchr(middleware->current_path, '/');
            if (last_slash && last_slash != middleware->current_path) {
                *last_slash = '\0';
            } else {
                strcpy(middleware->current_path, "/");
            }
            
            return 0;
        } else {
            // Đã ở thư mục gốc
            return 0;
        }
    }
    
    // Tìm thư mục theo đường dẫn
    FileNode* target = fat_driver_find_path(middleware->fat_driver, path);
    
    if (!target) {
        print_error("Directory not found: %s\n", path);
        return -1;
    }
    
    if (target->type != FILE_TYPE_DIRECTORY) {
        print_error("Not a directory: %s\n", path);
        return -1;
    }
    
    middleware->current_directory = target;
    
    // Cập nhật đường dẫn hiện tại
    if (path[0] == '/') {
        // Đường dẫn tuyệt đối
        strncpy(middleware->current_path, path, sizeof(middleware->current_path) - 1);
        middleware->current_path[sizeof(middleware->current_path) - 1] = '\0'; // Đảm bảo kết thúc chuỗi
    } else {
        // Đường dẫn tương đối
        size_t current_len = strlen(middleware->current_path);
        size_t path_len = strlen(path);
        if (current_len + 1 + path_len < sizeof(middleware->current_path)) {
            if (strcmp(middleware->current_path, "/") == 0) {
                strncat(middleware->current_path, path, sizeof(middleware->current_path) - current_len - 1);
            } else {
                strncat(middleware->current_path, "/", sizeof(middleware->current_path) - current_len - 1);
                strncat(middleware->current_path, path, sizeof(middleware->current_path) - current_len - 1);
            }
        } else {
            print_error("Path is too long\n");
            return -1;
        }
    }
    
    // Chuẩn hóa đường dẫn (xóa các dấu / thừa)
    char* p = middleware->current_path;
    char* q = middleware->current_path;
    
    while (*p) {
        if (*p == '/' && *(p+1) == '/') {
            p++;
        } else {
            *q++ = *p++;
        }
    }
    *q = '\0';
    
    return 0;
}

int middleware_cat(Middleware* middleware, const char* path) {
    if (!middleware || !path) return -1;
    
    // Tìm file theo đường dẫn
    FileNode* file = fat_driver_find_path(middleware->fat_driver, path);
    
    if (!file) {
        print_error("File not found: %s\n", path);
        return -1;
    }
    
    if (file->type != FILE_TYPE_REGULAR) {
        print_error("Not a regular file: %s\n", path);
        return -1;
    }
    
    // Đọc nội dung file
    uint8_t* buffer = malloc(file->size + 1);
    if (!buffer) {
        print_error("Memory allocation failed\n");
        return -1;
    }
    
    int bytes_read = fat_driver_read_file(middleware->fat_driver, file, buffer, file->size);
    if (bytes_read < 0) {
        print_error("Failed to read file: %s\n", path);
        free(buffer);
        return -1;
    }
    
    // Thêm ký tự kết thúc chuỗi
    buffer[bytes_read] = '\0';
    
    // In nội dung file
    printf("%s\n", buffer);
    
    free(buffer);
    return 0;
}

int middleware_evidence(Middleware* middleware) {
    if (!middleware) return -1;
    
    FATDriver* driver = middleware->fat_driver;
    if (!driver) return -1;
    
    // Hiển thị thông tin hệ thống tệp
    print_info("File System Information:\n");
    
    // Loại FAT
    const char* fat_type_str = "Unknown";
    switch (fat_driver_get_fat_type(driver)) {
        case FAT_TYPE_12: fat_type_str = "FAT12"; break;
        case FAT_TYPE_16: fat_type_str = "FAT16"; break;
        case FAT_TYPE_32: fat_type_str = "FAT32"; break;
        default: break;
    }
    printf("FAT Type: %s\n", fat_type_str);
    
    // Thông tin từ boot sector
    printf("Bytes per Sector: %u\n", driver->boot_sector.bytes_per_sector);
    printf("Sectors per Cluster: %u\n", driver->boot_sector.sectors_per_cluster);
    printf("Reserved Sectors: %u\n", driver->boot_sector.reserved_sectors);
    printf("Number of FATs: %u\n", driver->boot_sector.number_of_fats);
    
    if (fat_driver_get_fat_type(driver) != FAT_TYPE_32) {
        printf("Root Entry Count: %u\n", driver->boot_sector.root_entry_count);
    }
    
    uint32_t total_sectors = driver->boot_sector.total_sectors_16 ? 
                            driver->boot_sector.total_sectors_16 : 
                            driver->boot_sector.total_sectors_32;
    printf("Total Sectors: %u\n", total_sectors);
    
    uint32_t fat_size = driver->boot_sector.fat_size_16 ? 
                        driver->boot_sector.fat_size_16 : 
                        driver->boot_sector.fat_size_32;
    printf("FAT Size: %u sectors\n", fat_size);
    
    if (fat_driver_get_fat_type(driver) == FAT_TYPE_32) {
        printf("Root Cluster: %u\n", driver->boot_sector.root_cluster);
    }
    
    // Thông tin về kích thước
    uint64_t total_size, free_size;
    if (fat_driver_get_filesystem_info(driver, &total_size, &free_size) == 0) {
        printf("Total Size: %llu bytes\n", (unsigned long long)total_size);
        printf("Free Size: %llu bytes\n", (unsigned long long)free_size);
        printf("Used Size: %llu bytes\n", (unsigned long long)(total_size - free_size));
    }
    
    // Thông tin về cấu hình
    printf("\nConfiguration:\n");
    
    const char* mode_str = "Unknown";
    switch (driver->config.mode) {
        case MODE_READ_ONLY: mode_str = "Read-Only"; break;
        case MODE_READ_WRITE: mode_str = "Read-Write"; break;
    }
    printf("Mode: %s\n", mode_str);
    
    printf("Sector Size: %u\n", (uint32_t)driver->config.sector_size);
    printf("Cache Size: %u sectors\n", (uint32_t)driver->config.cache_size);
    printf("Directory Name Length: %u\n", (uint32_t)driver->config.dir_name_len);
    
    return 0;
}

int middleware_switch_to_root_mode(Middleware* middleware) {
    if (!middleware) return -1;
    
    middleware->is_root_mode = true;
    middleware->current_directory = fat_driver_get_root_directory(middleware->fat_driver);
    strcpy(middleware->current_path, "/");
    
    print_info("Switched to root mode\n");
    return 0;
}

int middleware_switch_to_user_mode(Middleware* middleware) {
    if (!middleware) return -1;
    
    middleware->is_root_mode = false;
    
    print_info("Switched to user mode\n");
    return 0;
}

const char* middleware_get_current_path(Middleware* middleware) {
    if (!middleware) return NULL;
    
    return middleware->current_path;
}

bool middleware_is_root_mode(Middleware* middleware) {
    if (!middleware) return false;
    
    return middleware->is_root_mode;
}
