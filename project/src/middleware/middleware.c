/**
 * @file middleware.c
 * @brief Middleware implementation
 * @details This file contains the implementation of middleware functions.
 * @date 2023-10-15
 * @author Le Duc Son
 */

#include "middleware.h"
#include "../utilities/log/print_color.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Initialize the middleware */
int middleware_init(Middleware* middleware) {
    FATDriver* fat_driver = malloc(sizeof(FATDriver)); /** Allocate FATDriver structure */
    if (!middleware || !fat_driver) return -1;
    
    /** Check input parameters */
    if (!middleware->img_path) {
        print_error("Image file path is required\n");
        return -1;
    }
    
    const char* ext = strrchr(middleware->img_path, '.');
    if (!ext || (strcmp(ext, ".img") != 0 && strcmp(ext, ".bin") != 0)) {
        print_error("File is not a supported image file (.img/.bin): %s\n", middleware->img_path);
        return -1;
    }
    
    /** Configure file system */
    FileSystemConfig config;
    config.img_path = middleware->img_path;
    config.mode = middleware->mode;
    config.fat_type = FAT_TYPE_16; /** Default, will be determined in fat_driver_mount */
    config.sector_size = SECTOR_SIZE_512;
    config.cache_size = CACHE_SIZE_16;
    config.dir_name_len = DIR_NAME_LEN_8;
    
    middleware->fat_driver = fat_driver;
    
    if (fat_driver_init(fat_driver, config) != 0) {
        print_error("Failed to initialize FAT Driver\n");
        free(fat_driver);
        return -1;
    }
    
    if (fat_driver_mount(fat_driver) != 0) {
        print_error("Failed to mount file system\n");
        fat_driver_unmount(fat_driver);
        fat_driver_deinit(fat_driver);
        free(fat_driver);
        return -1;
    }
    
    print_success("Mount successful\n");
    
    middleware->current_directory = fat_driver_get_root_directory(fat_driver);
    strcpy(middleware->current_path, "/");
    middleware->is_root_mode = true;
    
    /** Switch mode based on path */
    if (strcmp(middleware->current_path, "/") == 0) {
        middleware_switch_to_root_mode(middleware);
    } else {
        middleware_switch_to_user_mode(middleware);
    }
    return 0;
}

/** Deinitialize the middleware */
int middleware_denit(Middleware* middleware) {
    if (!middleware) return -1;
    
    if (middleware->fat_driver) {
        fat_driver_unmount(middleware->fat_driver);
        fat_driver_deinit(middleware->fat_driver);
        free(middleware->fat_driver);
        middleware->fat_driver = NULL;
    }
    
    return 0;
}

/** List directory contents */
int middleware_ls(Middleware* middleware, const char* path) {
    if (!middleware || !middleware->fat_driver) return -1;
    
    FileNode* target_dir = middleware->current_directory;
    
    /* If path is provided, find the target directory */
    if (path != NULL && strlen(path) > 0) {
        target_dir = fat_driver_find_path(middleware->fat_driver, path);
        if (!target_dir) {
            print_error("ls: %s: No such file or directory\n", path);
            return -1;
        }
        if (target_dir->type != FILE_TYPE_DIRECTORY) {
            /* If it's a file, we could just list it, but usually ls [path] on a file lists the file itself.
               For simplicity let's require a directory or handle file listing. */
            printf("%-32s %-12s %-12u\n", target_dir->name, "File", target_dir->size);
            return 0;
        }
    }
    
    if (!target_dir) return -1;
    
    /* Populate directory children before listing */
    if (fat_driver_list_directory(middleware->fat_driver, target_dir) != 0) {
        print_error("Failed to read directory contents\n");
        return -1;
    }
    
    FileNode* current = target_dir->children;
    
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
            snprintf(created, sizeof(created), "%04d-%02d-%02d %02d:%02d",
                    current->created_time.year, current->created_time.month, current->created_time.day,
                    current->created_time.hour, current->created_time.minute);
        } else {
            strcpy(created, "N/A");
        }
        
        if (current->modified_time.year > 0) {
            snprintf(modified, sizeof(modified), "%04d-%02d-%02d %02d:%02d",
                    current->modified_time.year, current->modified_time.month, current->modified_time.day,
                    current->modified_time.hour, current->modified_time.minute);
        } else {
            strcpy(modified, "N/A");
        }
        
        /** Print directory/file name and info safely */
        printf("%-32s %-12s %-12u %-20s %-20s\n", 
               current->name, type_str, current->size, created, modified);
               
        current = current->next;
    }
    
    return 0;
}

/** Change directory */
int middleware_cd(Middleware* middleware, const char* path) {
    if (!middleware || !path) return -1;
    
    /** Handle special cases */
    if (strcmp(path, "/") == 0) {
        /** Change to root directory */
        middleware->current_directory = fat_driver_get_root_directory(middleware->fat_driver);
        strcpy(middleware->current_path, "/");
        return 0;
    } else if (strcmp(path, ".") == 0 || strcmp(path, "./") == 0) {
        /** Stay in current directory */
        return 0;
    } else if (strcmp(path, "..") == 0 || strcmp(path, "../") == 0) {
        /** Change to parent directory */
        if (middleware->current_directory->parent) {
            middleware->current_directory = middleware->current_directory->parent;
            
            /** Update current path */
            char* last_slash = strrchr(middleware->current_path, '/');
            if (last_slash && last_slash != middleware->current_path) {
                *last_slash = '\0';  // Remove last directory
            } else {
                strcpy(middleware->current_path, "/");  // Stay at root
            }
            return 0;
        } else {
            /** Already at root directory */
            return 0;
        }
    }

    /** Copy current path for manipulation */
    char temp_path[PATH_MAX];
    strcpy(temp_path, middleware->current_path);

    /** Append path to current path */
    if (middleware->current_path[strlen(middleware->current_path) - 1] != '/') {
        strcat(temp_path, "/");
    }
    strcat(temp_path, path);

    /** Normalize path: Process "..", ".", and remove redundant slashes */
    char normalized_path[PATH_MAX];
    char* tokens[PATH_MAX / 2];
    int token_count = 0;
    
    char* token = strtok(temp_path, "/");
    while (token != NULL) {
        if (strcmp(token, "..") == 0) {
            /** Move up one level */
            if (token_count > 0) {
                token_count--;  // Remove last directory
            }
        } else if (strcmp(token, ".") != 0 && strcmp(token, "") != 0) {
            /** Normal directory */
            tokens[token_count++] = token;
        }
        token = strtok(NULL, "/");
    }

    /** Construct normalized path */
    if (token_count == 0) {
        strcpy(normalized_path, "/");
    } else {
        strcpy(normalized_path, "");
        for (int i = 0; i < token_count; i++) {
            strcat(normalized_path, "/");
            strcat(normalized_path, tokens[i]);
        }
    }

    /** Find target directory */
    FileNode* target = fat_driver_find_path(middleware->fat_driver, normalized_path);
    
    if (!target) {
        print_error("Directory not found: %s\n", path);
        return -1;
    }
    
    if (target->type != FILE_TYPE_DIRECTORY) {
        print_error("Not a directory: %s\n", path);
        return -1;
    }
    
    /** Update directory and path */
    middleware->current_directory = target;
    strncpy(middleware->current_path, normalized_path, sizeof(middleware->current_path) - 1);
    middleware->current_path[sizeof(middleware->current_path) - 1] = '\0'; /** Ensure null-terminated */

    return 0;
}

/** Concatenate and display file content */
int middleware_cat(Middleware* middleware, const char* path) {
    if (!middleware || !path) return -1;
    
    /** Update temporary path */
    char temp_path[256] = {0};
    strcpy(temp_path, middleware->current_path);
    if(middleware->current_path[strlen(middleware->current_path) - 1] != '/')
    {
        strcat(temp_path, "/");
    }
    strcat(temp_path, path);
    /** Find file by path */
    FileNode* file = fat_driver_find_path(middleware->fat_driver, temp_path);
    
    if (!file) {
        print_error("File not found: %s\n", path);
        return -1;
    }
    
    if (file->type != FILE_TYPE_REGULAR) {
        print_error("Not a regular file: %s\n", path);
        return -1;
    }
    
    /** Read file content */
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
    
    /** Add null-terminator */
    buffer[bytes_read] = '\0';
    
    /** Print file content */
    for (int i = 0; i < bytes_read; i++)
    {
        print_color(COLOR_YELLOW, "%c", buffer[i]);
    }
    printf("\n");
    fflush(stdout);
    
    free(buffer);
    return 0;
}

/** Display file system evidence */
int middleware_evidence(Middleware* middleware) {
    if (!middleware) return -1;
    
    FATDriver* driver = middleware->fat_driver;
    if (!driver) return -1;
    
    /** Display file system information */
    print_info("File System Information:\n");
    
    /** FAT Type */
    const char* fat_type_str = "Unknown";
    switch (fat_driver_get_fat_type(driver)) {
        case FAT_TYPE_12: fat_type_str = "FAT12"; break;
        case FAT_TYPE_16: fat_type_str = "FAT16"; break;
        case FAT_TYPE_32: fat_type_str = "FAT32"; break;
        default: break;
    }
    printf("FAT Type: %s\n", fat_type_str);
    
    /** Boot sector information */
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
    
    /** Size information */
    uint64_t total_size, free_size;
    if (fat_driver_get_filesystem_info(driver, &total_size, &free_size) == 0) {
        printf("Total Size: %llu bytes\n", (unsigned long long)total_size);
        printf("Free Size: %llu bytes\n", (unsigned long long)free_size);
        printf("Used Size: %llu bytes\n", (unsigned long long)(total_size - free_size));
    }
    
    /** Configuration information */
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

/** Switch to root mode */
int middleware_switch_to_root_mode(Middleware* middleware) {
    if (!middleware) return -1;
    
    middleware->is_root_mode = true;
    middleware->current_directory = fat_driver_get_root_directory(middleware->fat_driver);
    strcpy(middleware->current_path, "/");
    
    print_info("Switched to root mode\n");
    return 0;
}

/** Switch to user mode */
int middleware_switch_to_user_mode(Middleware* middleware) {
    if (!middleware) return -1;
    
    middleware->is_root_mode = false;
    
    print_info("Switched to user mode\n");
    return 0;
}

/** Get current path */
const char* middleware_get_current_path(Middleware* middleware) {
    if (!middleware) return NULL;
    
    return middleware->current_path;
}

/** Check if in root mode */
bool middleware_is_root_mode(Middleware* middleware) {
    if (!middleware) return false;
    return middleware->is_root_mode;
}

void middleware_display_prompt(Middleware* middleware) {
    if (!middleware) return;

    if (middleware->is_root_mode) {
        print_color(COLOR_BOLD COLOR_UNDERLINE COLOR_GREEN, "DEESOL");
        print_color(COLOR_BOLD, "@");
        print_color(COLOR_ITALIC COLOR_GREEN, "root: ");
    } else {
        print_color(COLOR_BOLD COLOR_UNDERLINE COLOR_GREEN, "DEESOL");
        print_color(COLOR_BOLD, "@");
        print_color(COLOR_ITALIC COLOR_BLUE, "user: ");
    }
    print_color(COLOR_MAGENTA, "%s", middleware->current_path);
    print_color(COLOR_YELLOW, "$> ");
    fflush(stdout);
}

