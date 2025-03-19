/**
 * @file fat_driver.c
 * @brief Initialize the FATDriver with the given configuration.
 * @date 2023-10-15
 * @author Le Duc Son
 */
#include "fat_driver.h"
#include "fat_driver_private.h"
#include <stdlib.h>
#include <string.h>

/* Local functions */
static int fat_driver_load_fat_table(FATDriver* driver);
static int fat_driver_load_root_directory(FATDriver* driver);
static int fat_driver_build_directory_tree(FATDriver* driver);
static void fat_driver_parse_boot_sector(FATDriver* driver, const uint8_t* boot_sector_buffer);

/**
 * Initialize the FATDriver with the given configuration.
 * 
 * This function initializes the hardware abstraction layer (HAL) and sets up
 * the FATDriver structure with the specified file system configuration. It also
 * allocates memory for the cache based on the provided configuration.
 * 
 * @param driver Pointer to the FATDriver structure to initialize.
 * @param config The file system configuration containing parameters such as 
 *               image path and cache size.
 * @return 0 if the initialization is successful, -1 if there is any failure 
 *         during the process.
 */
int fat_driver_init(FATDriver* driver, const FileSystemConfig config) {
    /* Allocate memory for HAL */
    HAL* hal = malloc(sizeof(HAL));
    if (hal_init(hal, config.img_path, SECTOR_SIZE_512) != 0) {
        return -1;
    }
    if (!driver || !hal) return -1;

    /* Initialize driver components */
    memset(driver, 0, sizeof(FATDriver));
    driver->hal = hal;
    driver->config = config;
    
    /* Allocate memory for cache */
    driver->cache_size = (uint32_t)config.cache_size;
    driver->cache = malloc(driver->cache_size * hal_get_sector_size(hal));
    if (!driver->cache) return -1;
    
    return 0;
}
/**
 * Deinitializes the FATDriver and releases all the allocated resources.
 * 
 * This function is the counterpart of fat_driver_init() and should be called
 * when the FATDriver is no longer needed. It releases all the allocated resources
 * and deinitializes the hardware abstraction layer (HAL).
 * 
 * @param driver Pointer to the FATDriver structure to deinitialize.
 * @return 0 if the deinitialization is successful, -1 if there is any failure 
 *         during the process.
 */
int fat_driver_deinit(FATDriver* driver) {
    if (!driver || !driver->hal) return -1;
    
    hal_deinit(driver->hal);
    
    return 0;
}

/**
 * Mounts the FAT file system and sets up the FATDriver for operations.
 * 
 * This function mounts the FAT file system and sets up the FATDriver for 
 * operations. It reads the boot sector, parses it to extract the necessary 
 * configuration parameters, calculates the sector numbers of the FAT, root 
 * directory and data area, loads the FAT table, loads the root directory and 
 * builds the directory tree.
 * 
 * @param driver Pointer to the FATDriver structure to mount.
 * @return 0 if the mount is successful, -1 if there is any failure during the 
 *         process.
 */
int fat_driver_mount(FATDriver* driver) {
    if (!driver || !driver->hal) return -1;
    
    uint8_t* boot_sector_buffer = malloc(hal_get_sector_size(driver->hal));
    if (!boot_sector_buffer) return -1;
    
    /* Đọc boot sector */
    uint32_t bytes_read = hal_read_sector(driver->hal, 0, boot_sector_buffer);
    if (bytes_read != (uint32_t)hal_get_sector_size(driver->hal)) {
        free(boot_sector_buffer);
        return -1;
    }
    
    /* Phân tích boot sector */
    fat_driver_parse_boot_sector(driver, boot_sector_buffer);
    free(boot_sector_buffer);
    
    /* Tính toán các thông số cần thiết */
    driver->first_fat_sector = driver->boot_sector.reserved_sectors;
    
    /* Tính số sector của thư mục gốc (chỉ áp dụng cho FAT12/16) */
    driver->root_dir_sectors = ((driver->boot_sector.root_entry_count * 32) + 
                               (driver->boot_sector.bytes_per_sector - 1)) / 
                               driver->boot_sector.bytes_per_sector;
    
    /* Tính sector đầu tiên của thư mục gốc */
    driver->first_root_dir_sector = driver->boot_sector.reserved_sectors + 
                                   (driver->boot_sector.number_of_fats * 
                                   (driver->boot_sector.fat_size_16 ? 
                                    driver->boot_sector.fat_size_16 : 
                                    driver->boot_sector.fat_size_32));
    
    /* Tính sector đầu tiên của vùng dữ liệu */
    if (fat_driver_get_fat_type(driver) == FAT_TYPE_32) {
        driver->first_data_sector = driver->boot_sector.reserved_sectors + 
                                   (driver->boot_sector.number_of_fats * 
                                    driver->boot_sector.fat_size_32);
    } else {
        driver->first_data_sector = driver->first_root_dir_sector + 
                                   driver->root_dir_sectors;
    }
    
    /* Tính tổng số sector dữ liệu */
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
    
    /* Tính tổng số cluster */
    driver->total_clusters = driver->data_sectors / 
                            driver->boot_sector.sectors_per_cluster;
    
    /* Load bảng FAT */
    if (fat_driver_load_fat_table(driver) != 0) {
        return -1;
    }
    
    /* Load thư mục gốc */
    if (fat_driver_load_root_directory(driver) != 0) {
        return -1;
    }
    
    /* Xây dựng cây thư mục */
    if (fat_driver_build_directory_tree(driver) != 0) {
        return -1;
    }
    
    /* Đặt thư mục hiện tại là thư mục gốc */
    driver->current_directory = driver->root_directory;
    
    return 0;
}

/**
 * Unmounts the FAT file system and releases all the allocated resources.
 * 
 * This function is the counterpart of fat_driver_mount() and should be called
 * when the FATDriver is no longer needed. It releases all the allocated resources
 * and deinitializes the FATDriver.
 * 
 * @param driver Pointer to the FATDriver structure to unmount.
 * @return 0 if the unmount is successful, -1 if there is any failure during the 
 *         process.
 */
void fat_driver_unmount(FATDriver* driver) {
    if (!driver) return;
    
    /* Giải phóng bộ nhớ */
    if (driver->fat_table) {
        free(driver->fat_table);
        driver->fat_table = NULL;
    }
    
    if (driver->cache) {
        free(driver->cache);
        driver->cache = NULL;
    }
    
    /* Giải phóng cây thư mục */
    if (driver->root_directory) {
        fat_driver_free_file_node(driver->root_directory);
        driver->root_directory = NULL;
    }
    
    driver->current_directory = NULL;
}
/**
 * Gets the root directory of the FAT file system.
 * 
 * This function returns a pointer to the root directory of the FAT file system.
 * 
 * @param driver Pointer to the FATDriver structure.
 * @return Pointer to the root directory if successful, NULL if failed.
 */
FileNode* fat_driver_get_root_directory(FATDriver* driver) {
    if (!driver) return NULL;
    
    return driver->root_directory;
}

/**
 * Gets the current directory of the FAT file system.
 * 
 * This function returns a pointer to the current directory of the FAT file system.
 * 
 * @param driver Pointer to the FATDriver structure.
 * @return Pointer to the current directory if successful, NULL if failed.
 */
FileNode* fat_driver_get_current_directory(FATDriver* driver) {
    if (!driver) return NULL;
    
    return driver->current_directory;
}

/**
 * Sets the current directory of the FAT file system.
 * 
 * This function sets the current directory of the FAT file system to the given
 * directory. It returns 0 if successful or -1 if failed.
 * 
 * @param driver Pointer to the FATDriver structure.
 * @param directory Pointer to the directory to set as the current directory.
 * @return 0 if successful, -1 if failed.
 */
int fat_driver_set_current_directory(FATDriver* driver, FileNode* directory) {
    if (!driver || !directory || directory->type != FILE_TYPE_DIRECTORY) {
        return -1;
    }
    
    driver->current_directory = directory;
    return 0;
}

/**
 * Finds a path in the FAT file system.
 * 
 * This function takes a path and returns a pointer to the FileNode if successful
 * or NULL if failed.
 * 
 * @param driver Pointer to the FATDriver structure.
 * @param path Path to find.
 * @return Pointer to the FileNode if successful, NULL if failed.
 */
FileNode* fat_driver_find_path(FATDriver* driver, const char* path) {
    if (!driver || !path) return NULL;
    
    /* Handle absolute path */
    if (path[0] == '/') {
        if (path[1] == '\0') {
            return driver->root_directory;
        }
        
        /* Skip the leading '/' character */
        path++;
        FileNode* current = driver->root_directory;
        return fat_driver_find_path_recursive(current, path);
    }
    
    /* Handle relative path */
    FileNode* current = driver->current_directory;
    return fat_driver_find_path_recursive(current, path);
}

/**
 * Recursive function to find a path.
 * 
 * This function takes a path and returns a pointer to the FileNode if successful
 * or NULL if failed.
 * 
 * @param current Current directory.
 * @param path Path to find.
 * @return Pointer to the FileNode if successful, NULL if failed.
 */
FileNode* fat_driver_find_path_recursive(FileNode* current, const char* path) {
    if (!current || !path || path[0] == '\0') {
        return current;
    }

    /**
     * Separate the first component of the path.
     */
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

    /**
     * Handle special cases.
     */
    if (strcmp(component, ".") == 0) {
        return fat_driver_find_path_recursive(current, next_path);
    } else if (strcmp(component, "..") == 0) {
        if (current->parent) {
            return fat_driver_find_path_recursive(current->parent, next_path);
        } else {
            return fat_driver_find_path_recursive(current, next_path);
        }
    }

    /**
     * Search in the children list.
     */
    FileNode* child = current->children;
    while (child) {
        if (strcmp(child->name, component) == 0) {
            if (*next_path == '\0') {
                return child;
            } else if (child->type == FILE_TYPE_DIRECTORY) {
                return fat_driver_find_path_recursive(child, next_path);
            } else {
                return NULL; /* Cannot navigate into a file */
            }
        }
        child = child->next;
    }

    return NULL; /* Not found */
}

/**
 * Reads a file from the file system.
 * 
 * This function reads a file from the file system and stores its content in the
 * provided buffer. The size of the buffer must be at least as large as the size
 * of the file. If the file is larger than the buffer, the function will return
 * an error.
 * 
 * @param driver Pointer to the FATDriver structure.
 * @param file Pointer to the FileNode structure of the file to read.
 * @param buffer Buffer to store the file content.
 * @param size Size of the buffer.
 * @return The number of bytes read if successful, -1 if failed.
 */
int fat_driver_read_file(FATDriver* driver, FileNode* file, void* buffer, uint32_t size) {
    if (!driver || !file || !buffer || file->type != FILE_TYPE_REGULAR) {
        return -1;
    }
    
    /* Check mode */
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
            
            /** Get the next cluster */
            current_cluster = fat_driver_get_next_cluster(driver, current_cluster);
        }
        
        free(temp_buffer);
        return bytes_read;
    }
    
    return -1;
}

/**
 * Writes a file to the file system.
 * 
 * This function writes a file to the file system from the provided buffer. The
 * size of the buffer must be at least as large as the size of the file. If the
 * file is larger than the buffer, the function will return an error.
 * 
 * @param driver Pointer to the FATDriver structure.
 * @param file Pointer to the FileNode structure of the file to write.
 * @param buffer Buffer containing the file content.
 * @param size Size of the buffer.
 * @return The number of bytes written if successful, -1 if failed.
 */
int fat_driver_write_file(FATDriver* driver, FileNode* file, const void* buffer, uint32_t size) {
    if (!driver || !file || !buffer || file->type != FILE_TYPE_REGULAR) {
        return -1;
    }
    
    /** Check mode */
    if (driver->config.mode == MODE_READ_WRITE) {
        /** Implement writing a file */
        // ...
        
        return size;
    }
    
    return -1;
}

/**
 * Gets the FAT type of the file system.
 * 
 * This function determines the FAT type of the file system based on the number
 * of clusters.
 * 
 * @param driver Pointer to the FATDriver structure.
 * @return The FAT type of the file system.
 */
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

/**
 * Gets the file system information.
 * 
 * This function gets the total size and free size of the file system.
 * 
 * @param driver Pointer to the FATDriver structure.
 * @param total_size Pointer to a uint64_t to store the total size of the file system.
 * @param free_size Pointer to a uint64_t to store the free size of the file system.
 * @return 0 if successful, -1 if failed.
 */
int fat_driver_get_filesystem_info(FATDriver* driver, uint64_t* total_size, uint64_t* free_size) {
    if (!driver || !total_size || !free_size) return -1;
    
    uint32_t cluster_size = driver->boot_sector.sectors_per_cluster * 
                           driver->boot_sector.bytes_per_sector;
    
    *total_size = (uint64_t)driver->total_clusters * cluster_size;
    
    /**
     * Counts the number of free clusters.
     */
    uint32_t free_clusters = 0;
    for (uint32_t i = 2; i < driver->total_clusters + 2; i++) {
        if (fat_driver_get_fat_entry(driver, i) == 0) {
            free_clusters++;
        }
    }
    
    *free_size = (uint64_t)free_clusters * cluster_size;
    
    return 0;
}

/**
 * Converts a cluster number to a sector number.
 * 
 * This function converts a cluster number to a sector number based on the 
 * configuration of the FAT file system.
 * 
 * @param driver Pointer to the FATDriver structure.
 * @param cluster Cluster number to convert.
 * @return The sector number corresponding to the given cluster number.
 */
uint32_t fat_driver_cluster_to_sector(FATDriver* driver, uint32_t cluster) {
    if (!driver || cluster < 2) return 0;
    
    return driver->first_data_sector + 
          (cluster - 2) * driver->boot_sector.sectors_per_cluster;
}

/**
 * Frees a FileNode structure and all its children.
 * 
 * This function frees a FileNode structure and all its children.
 * 
 * @param node Pointer to the FileNode structure to free.
 */
void fat_driver_free_file_node(FileNode* node) {
    if (!node) return;
    
    /** Frees the children first */
    FileNode* child = node->children;
    while (child) {
        FileNode* next = child->next;
        fat_driver_free_file_node(child);
        child = next;
    }
    
    /** Free the current node */
    free(node);
}

/** Internal function to parse the boot sector */
static void fat_driver_parse_boot_sector(FATDriver* driver, const uint8_t* boot_sector_buffer) {
    if (!driver || !boot_sector_buffer) return;
    
    BootSector* bs = &driver->boot_sector;
    
    /* Copy fields from boot sector */
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
    
    /* Check if it is FAT32 */
    if (bs->fat_size_16 == 0) {
        bs->fat_size_32 = *(uint32_t*)(boot_sector_buffer + 36);
        bs->extended_flags = *(uint16_t*)(boot_sector_buffer + 40);
        bs->fs_version = *(uint16_t*)(boot_sector_buffer + 42);
        bs->root_cluster = *(uint32_t*)(boot_sector_buffer + 44);
        bs->fs_info = *(uint16_t*)(boot_sector_buffer + 48);
        bs->backup_boot_sector = *(uint16_t*)(boot_sector_buffer + 50);
        /* Copy 12 reserved bytes */
        memcpy(bs->reserved, boot_sector_buffer + 52, 12);
    }
    
    /* Copy common fields */
    bs->drive_number = *(uint8_t*)(boot_sector_buffer + (bs->fat_size_16 == 0 ? 64 : 36));
    bs->reserved1 = *(uint8_t*)(boot_sector_buffer + (bs->fat_size_16 == 0 ? 65 : 37));
    bs->boot_signature = *(uint8_t*)(boot_sector_buffer + (bs->fat_size_16 == 0 ? 66 : 38));
    bs->volume_id = *(uint32_t*)(boot_sector_buffer + (bs->fat_size_16 == 0 ? 67 : 39));
    
    /* Copy volume label (11 bytes) */
    memcpy(bs->volume_label, boot_sector_buffer + (bs->fat_size_16 == 0 ? 71 : 43), 11);
    /* bs->volume_label[11] = '\0'; */
    
    /* Copy file system type (8 bytes) */
    memcpy(bs->fs_type, boot_sector_buffer + (bs->fat_size_16 == 0 ? 82 : 54), 8);
    /* bs->fs_type[8] = '\0'; */
}

/**
 * Internal function to load the FAT table.
 */
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

/**
 * Internal function to load the root directory.
 */
static int fat_driver_load_root_directory(FATDriver* driver) {
    if (!driver) return -1;
    
    /* Create node for the root directory */
    driver->root_directory = malloc(sizeof(FileNode));
    if (!driver->root_directory) return -1;
    
    memset(driver->root_directory, 0, sizeof(FileNode));
    strcpy(driver->root_directory->name, "/");
    driver->root_directory->type = FILE_TYPE_DIRECTORY;
    driver->root_directory->attributes.directory = true; /* FAT_ATTR_DIRECTORY; */
    
    if (fat_driver_get_fat_type(driver) == FAT_TYPE_32) {
        driver->root_directory->first_cluster = driver->boot_sector.root_cluster;
    } else {
        driver->root_directory->first_cluster = 0; /* Root directory in FAT12/16 is not in the data area */
    }
    
    return 0;
}

/**
 * Internal function to build the directory tree.
 */
static int fat_driver_build_directory_tree(FATDriver* driver) {
    if (!driver || !driver->root_directory) return -1;
    
    uint32_t sector_size = hal_get_sector_size(driver->hal);
    uint8_t* buffer = malloc(sector_size);
    if (!buffer) return -1;
    
    /**
     * Process the root directory
     */
    if (fat_driver_get_fat_type(driver) == FAT_TYPE_32) {
        /* In FAT32, the root directory is a cluster chain */
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
                
                /* Process the entries in the sector */
                for (uint32_t j = 0; j < sector_size; j += 32) {
                    FATDirEntry* entry = (FATDirEntry*)(buffer + j);
                    
                    /* Check for empty or deleted entries */
                    if (entry->name[0] == 0x00 || entry->name[0] == (uint8_t)0xE5) {
                        continue;
                    }
                    
                    /* Skip volume label entries */
                    if (entry->attributes & FAT_ATTR_VOLUME_ID) {
                        continue;
                    }
                    
                    /* Create a new node */
                    FileNode* node = malloc(sizeof(FileNode));
                    if (!node) {
                        free(buffer);
                        return -1;
                    }
                    
                    /* Fill in the node */
                    fat_driver_fill_file_node(driver, node, entry);
                    
                    /* Add the node to the root directory */
                    node->parent = driver->root_directory;
                    node->next = driver->root_directory->children;
                    driver->root_directory->children = node;
                }
            }
            
            /* Get the next cluster */
            current_cluster = fat_driver_get_next_cluster(driver, current_cluster);
        }
    } else {
        /* In FAT12/16, the root directory is at a fixed location */
        for (uint32_t i = 0; i < driver->root_dir_sectors; i++) {
            uint32_t read_bytes = hal_read_sector(driver->hal, driver->first_root_dir_sector + i, buffer);
            if (read_bytes != sector_size) {
                free(buffer);
                return -1;
            }
            
            /* Process the entries in the sector */
            for (uint32_t j = 0; j < sector_size; j += 32) {
                FATDirEntry* entry = (FATDirEntry*)(buffer + j);
                
                /* Check for empty or deleted entries */
                if (entry->name[0] == 0x00 || entry->name[0] == (uint8_t)0xE5) {
                    continue;
                }
                
                /* Skip volume label entries */
                if (entry->attributes & FAT_ATTR_VOLUME_ID) {
                    continue;
                }
                
                /* Create a new node */
                FileNode* node = malloc(sizeof(FileNode));
                if (!node) {
                    free(buffer);
                    return -1;
                }
                
                /* Fill in the node */
                fat_driver_fill_file_node(driver, node, entry);
                
                /* Add the node to the root directory */
                node->parent = driver->root_directory;
                node->next = driver->root_directory->children;
                driver->root_directory->children = node;
            }
        }
    }
    
    /* Process the subdirectories */
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

/* Recursive function to build the directory tree */
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
            
            /* Process the entries in the sector */
            for (uint32_t j = 0; j < sector_size; j += 32) {
                FATDirEntry* entry = (FATDirEntry*)(buffer + j);
                
                /* Check for empty or deleted entries */
                if (entry->name[0] == 0x00 || entry->name[0] == (uint8_t)0xE5) {
                    continue;
                }
                
                /* Skip volume label entries and . and .. entries */
                if ((entry->attributes & FAT_ATTR_VOLUME_ID) ||
                    (entry->name[0] == '.' && entry->name[1] == ' ') ||
                    (entry->name[0] == '.' && entry->name[1] == '.' && entry->name[2] == ' ')) {
                    continue;
                }
                
                /* Create a new node */
                FileNode* node = malloc(sizeof(FileNode));
                if (!node) {
                    free(buffer);
                    return -1;
                }
                
                /* Fill in the node */
                fat_driver_fill_file_node(driver, node, entry);
                
                /* Add the node to the current directory */
                node->parent = directory;
                node->next = directory->children;
                directory->children = node;
            }
        }
        
        /* Get the next cluster */
        current_cluster = fat_driver_get_next_cluster(driver, current_cluster);
    }
    
    /* Process the subdirectories */
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

/**
 * Fills in the node from the entry.
 */
void fat_driver_fill_file_node(FATDriver* driver, FileNode* node, const FATDirEntry* entry) {
    if (!driver || !node || !entry) return;
    
    memset(node, 0, sizeof(FileNode));
    
    /**
     * Convert the file name from the 8.3 format.
     */
    char name[13] = {0};
    int name_len = 0;
    
    /**
     * Process the name part (8 characters).
     */
    for (int i = 0; i < 8; i++) {
        if (entry->name[i] != ' ') {
            name[name_len++] = entry->name[i];
        }
    }
    
    /**
     * Process the extension part (3 characters).
     */
    if (entry->ext[0] != ' ') {
        name[name_len++] = '.';
        for (int i = 0; i < 3; i++) {
            if (entry->ext[i] != ' ') {
                name[name_len++] = entry->ext[i];
            }
        }
    }
    
    name[name_len] = '\0';
    
    /**
     * Convert the name to lowercase.
     */
    for (int i = 0; i < name_len; i++) {
        if (name[i] >= 'A' && name[i] <= 'Z') {
            name[i] = name[i] - 'A' + 'a';
        }
    }
    
    strcpy(node->name, name);
    
    /**
     * Fill in other information.
     */
    node->size = entry->file_size;
    node->attributes.directory = true; /* FAT_ATTR_DIRECTORY; */
    
    if (entry->attributes & FAT_ATTR_DIRECTORY) {
        node->type = FILE_TYPE_DIRECTORY;
    } else {
        node->type = FILE_TYPE_REGULAR;
    }
    
    /**
     * Calculate the first cluster.
     */
    node->first_cluster = entry->first_cluster_low;
    if (fat_driver_get_fat_type(driver) == FAT_TYPE_32) {
        node->first_cluster |= ((uint32_t)entry->first_cluster_high << 16);
    }
    
    /**
     * Fill in the time information.
     */
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

/* Function to get the value of an entry in the FAT table */
uint32_t fat_driver_get_fat_entry(FATDriver* driver, uint32_t cluster) {
    if (!driver || !driver->fat_table) return 0;
    
    FatType fat_type = fat_driver_get_fat_type(driver);
    
    if (fat_type == FAT_TYPE_12) {
        uint32_t fat_offset = cluster + (cluster / 2);
        uint16_t fat_entry = *(uint16_t*)((uint8_t*)driver->fat_table + fat_offset);
        
        if (cluster & 0x1) {
            /* Odd cluster */
            return fat_entry >> 4;
        } else {
            /* Even cluster */
            return fat_entry & 0x0FFF;
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

/**
 * Gets the next cluster in the cluster chain.
 * 
 * @param driver Pointer to the FATDriver structure.
 * @param cluster The current cluster.
 * @return The next cluster in the chain.
 */
uint32_t fat_driver_get_next_cluster(FATDriver* driver, uint32_t cluster) {
    if (!driver) return 0;
    
    return fat_driver_get_fat_entry(driver, cluster);
}
