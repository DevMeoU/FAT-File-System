/**
 * @file fat_driver.c
 * @brief Initialize the FATDriver with the given configuration.
 * @date 2023-10-15
 * @author Le Duc Son
 */
#include "fat_driver.h"
#include "fat_driver_private.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>

/* Local functions */
static int fat_driver_load_root_directory(FATDriver* driver);
static void fat_driver_parse_boot_sector(FATDriver* driver, const uint8_t* boot_sector_buffer);
static FileNode* fat_driver_find_in_directory(FATDriver* driver, FileNode* dir, const char* name);

/**
 * Initialize the FATDriver with the given configuration.
 */
int fat_driver_init(FATDriver* driver, const FileSystemConfig config) {
    /* Allocate memory for HAL */
    HAL* hal = malloc(sizeof(HAL));
    if (hal_init(hal, config.img_path, config.sector_size) != 0) {
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
    
    /* Allocate memory for FAT cache (1 sector) */
    driver->fat_cache = malloc(hal_get_sector_size(hal));
    if (!driver->fat_cache) {
        free(driver->cache);
        return -1;
    }
    driver->fat_cache_sector = 0xFFFFFFFF; /* Invalid sector to start */
    
    return 0;
}

/**
 * Deinitializes the FATDriver.
 */
int fat_driver_deinit(FATDriver* driver) {
    if (!driver || !driver->hal) return -1;
    
    hal_deinit(driver->hal);
    free(driver->hal);
    driver->hal = NULL;
    
    return 0;
}

/**
 * Mounts the FAT file system.
 */
int fat_driver_mount(FATDriver* driver) {
    if (!driver || !driver->hal) return -1;
    
    uint8_t* boot_sector_buffer = malloc(hal_get_sector_size(driver->hal));
    if (!boot_sector_buffer) return -1;
    
    /* Read boot sector */
    uint32_t bytes_read = hal_read_sector(driver->hal, 0, boot_sector_buffer);
    if (bytes_read != (uint32_t)hal_get_sector_size(driver->hal)) {
        printf("[DEBUG] Error: hal_read_sector failed. bytes_read=%u, expected=%u\n", bytes_read, hal_get_sector_size(driver->hal));
        fflush(stdout);
        free(boot_sector_buffer);
        return -1;
    }
    
    /* Parse boot sector */
    fat_driver_parse_boot_sector(driver, boot_sector_buffer);
    free(boot_sector_buffer);
    
    printf("[DEBUG] BPS: %u, SPC: %u, RS: %u, NF: %u, REC: %u, FS16: %u, FS32: %u\n",
           driver->boot_sector.bytes_per_sector,
           driver->boot_sector.sectors_per_cluster,
           driver->boot_sector.reserved_sectors,
           driver->boot_sector.number_of_fats,
           driver->boot_sector.root_entry_count,
           driver->boot_sector.fat_size_16,
           driver->boot_sector.fat_size_32);
    fflush(stdout);
    
    /* Validate critical parameters to avoid division by zero and invalid memory access */
    if (driver->boot_sector.bytes_per_sector == 0 || 
        driver->boot_sector.sectors_per_cluster == 0) {
        printf("[DEBUG] Error: Invalid critical parameters. BPS=%u, SPC=%u\n", 
               driver->boot_sector.bytes_per_sector, 
               driver->boot_sector.sectors_per_cluster);
        return -1;
    }
    
    /* Calculate necessary parameters */
    driver->first_fat_sector = driver->boot_sector.reserved_sectors;
    
    /* Calculate root dir sectors (FAT12/16) */
    driver->root_dir_sectors = ((driver->boot_sector.root_entry_count * 32) + 
                               (driver->boot_sector.bytes_per_sector - 1)) / 
                               driver->boot_sector.bytes_per_sector;
    
    /* Calculate first root dir sector */
    driver->first_root_dir_sector = driver->boot_sector.reserved_sectors + 
                                   (driver->boot_sector.number_of_fats * 
                                   (driver->boot_sector.fat_size_16 ? 
                                    driver->boot_sector.fat_size_16 : 
                                    driver->boot_sector.fat_size_32));
    
    /* Calculate first data sector */
    if (fat_driver_get_fat_type(driver) == FAT_TYPE_32) {
        driver->first_data_sector = driver->boot_sector.reserved_sectors + 
                                   (driver->boot_sector.number_of_fats * 
                                    driver->boot_sector.fat_size_32);
    } else {
        driver->first_data_sector = driver->first_root_dir_sector + 
                                   driver->root_dir_sectors;
    }
    
    /* Calculate total data sectors */
    uint32_t total_sectors = driver->boot_sector.total_sectors_16 ? 
                            driver->boot_sector.total_sectors_16 : 
                            driver->boot_sector.total_sectors_32;
    
    uint32_t metadata_sectors = (driver->boot_sector.reserved_sectors + 
                                (driver->boot_sector.number_of_fats * 
                                (driver->boot_sector.fat_size_16 ? 
                                 driver->boot_sector.fat_size_16 : 
                                 driver->boot_sector.fat_size_32)) + 
                                driver->root_dir_sectors);

    if (total_sectors < metadata_sectors) {
        printf("[DEBUG] Error: total_sectors (%u) < metadata_sectors (%u)\n", total_sectors, metadata_sectors);
        fflush(stdout);
        return -1; /* Invalid file system size */
    }
    
    driver->data_sectors = total_sectors - metadata_sectors;
    printf("[DEBUG] total_sectors: %u, metadata_sectors: %u, data_sectors: %u\n", total_sectors, metadata_sectors, driver->data_sectors);
    fflush(stdout);
    
    /* Calculate total clusters */
    driver->total_clusters = driver->data_sectors / 
                            driver->boot_sector.sectors_per_cluster;

    
    /* Initialize root directory (node only) */
    if (fat_driver_load_root_directory(driver) != 0) {
        printf("[DEBUG] Error: fat_driver_load_root_directory failed\n");
        return -1;
    }
    
    /* Set current directory to root */
    driver->current_directory = driver->root_directory;
    
    return 0;
}

/**
 * Unmounts the FAT file system.
 */
void fat_driver_unmount(FATDriver* driver) {
    if (!driver) return;
    
    /* Free memory */
    if (driver->fat_cache) {
        free(driver->fat_cache);
        driver->fat_cache = NULL;
    }
    
    if (driver->cache) {
        free(driver->cache);
        driver->cache = NULL;
    }
    
    /* Free root directory node */
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

/**
 * Finds a path in the FAT file system (On-Demand).
 */
FileNode* fat_driver_find_path(FATDriver* driver, const char* path) {
    if (!driver || !path) return NULL;
    
    FileNode* current = driver->current_directory;
    
    /* Handle absolute path */
    if (path[0] == '/') {
        current = driver->root_directory;
        while (path[0] == '/') path++; /* Skip slashes */
        if (path[0] == '\0') return current;
    }
    
    char component[FILE_NAME_MAX + 1];
    
    while (path[0] != '\0') {
        /* Extract next component */
        const char* next_slash = strchr(path, '/');
        size_t len = next_slash ? (size_t)(next_slash - path) : strlen(path);
        
        if (len > FILE_NAME_MAX) len = FILE_NAME_MAX;
        strncpy(component, path, len);
        component[len] = '\0';
        
        /* Advance path pointer */
        path += len;
        if (path[0] == '/') path++;
        
        /* Special cases */
        if (strcmp(component, ".") == 0) {
            continue;
        } else if (strcmp(component, "..") == 0) {
            // Since we don't have a tree with parent pointers effective for on-demand without keeping history,
            // this is tricky. If we are traversing down, we lose parent context unless we keep a stack or the node has it.
            // But we are creating nodes on the fly. 
            // For now, if we are at root, stay at root. 
            // NOTE: Implementing ".." correctly in on-demand requiring tracking or re-opening parent.
            // The existing `FileNode` has `parent` pointer. We can set it when we find a child.
            // But if we just loaded `current`, `current->parent` might be NULL if we didn't set it (e.g. root).
            if (current->parent) {
                current = current->parent;
            }
            continue;
        }
        
        /* Find component in current directory */
        FileNode* next_node = fat_driver_find_in_directory(driver, current, component);
        if (!next_node) {
            return NULL; /* Not found */
        }
        
        /* Setup parent pointer for the new node so ".." works */
        next_node->parent = current;
        
        /* If there is more path, next_node must be a directory */
        if (path[0] != '\0' && next_node->type != FILE_TYPE_DIRECTORY) {
            fat_driver_free_file_node(next_node);
            return NULL;
        }
        
        /* Move to next */
        // CAUTION: If we are just traversing, we might leak memory if we don't attach `next_node` to something or free it later.
        // The original code built a tree. Here we are returning a node.
        // If we want to return a node that is part of a tree, we should attach it to `current->children`?
        // Let's attach it to cache it.
        
        // Check if it's already in children to avoid duplicates?
        // simplified: Just return the new node. The caller manages it (or we leak if we don't track).
        // To be safe and mimic previous behavior: attach to current->children.
        
        next_node->next = current->children;
        current->children = next_node;
        
        current = next_node;
    }
    
    return current;
}

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
            
            current_cluster = fat_driver_get_next_cluster(driver, current_cluster);
        }
        
        free(temp_buffer);
        return bytes_read;
    }
    
    return -1;
}

int fat_driver_write_file(FATDriver* driver, FileNode* file, const void* buffer, uint32_t size) {
    /* Not implemented */
    (void)driver;
    (void)file;
    (void)buffer;
    (void)size;
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
    
    FileNode* child = node->children;
    while (child) {
        FileNode* next = child->next;
        fat_driver_free_file_node(child);
        child = next;
    }
    free(node);
}

static void fat_driver_parse_boot_sector(FATDriver* driver, const uint8_t* boot_sector_buffer) {
    if (!driver || !boot_sector_buffer) return;
    
    BootSector* bs = &driver->boot_sector;
    
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
    
    if (bs->fat_size_16 == 0) {
        bs->fat_size_32 = *(uint32_t*)(boot_sector_buffer + 36);
        bs->extended_flags = *(uint16_t*)(boot_sector_buffer + 40);
        bs->fs_version = *(uint16_t*)(boot_sector_buffer + 42);
        bs->root_cluster = *(uint32_t*)(boot_sector_buffer + 44);
        bs->fs_info = *(uint16_t*)(boot_sector_buffer + 48);
        bs->backup_boot_sector = *(uint16_t*)(boot_sector_buffer + 50);
        memcpy(bs->reserved, boot_sector_buffer + 52, 12);
    }
    
    bs->drive_number = *(uint8_t*)(boot_sector_buffer + (bs->fat_size_16 == 0 ? 64 : 36));
    bs->reserved1 = *(uint8_t*)(boot_sector_buffer + (bs->fat_size_16 == 0 ? 65 : 37));
    bs->boot_signature = *(uint8_t*)(boot_sector_buffer + (bs->fat_size_16 == 0 ? 66 : 38));
    bs->volume_id = *(uint32_t*)(boot_sector_buffer + (bs->fat_size_16 == 0 ? 67 : 39));
    
    memcpy(bs->volume_label, boot_sector_buffer + (bs->fat_size_16 == 0 ? 71 : 43), 11);
    memcpy(bs->fs_type, boot_sector_buffer + (bs->fat_size_16 == 0 ? 82 : 54), 8);
}

static int fat_driver_load_root_directory(FATDriver* driver) {
    if (!driver) return -1;
    
    driver->root_directory = malloc(sizeof(FileNode));
    if (!driver->root_directory) return -1;
    
    memset(driver->root_directory, 0, sizeof(FileNode));
    strcpy(driver->root_directory->name, "/");
    driver->root_directory->type = FILE_TYPE_DIRECTORY;
    driver->root_directory->attributes.directory = true;
    
    if (fat_driver_get_fat_type(driver) == FAT_TYPE_32) {
        driver->root_directory->first_cluster = driver->boot_sector.root_cluster;
    } else {
        driver->root_directory->first_cluster = 0;
    }
    
    return 0;
}

void fat_driver_fill_file_node(FATDriver* driver, FileNode* node, const FATDirEntry* entry) {
    if (!driver || !node || !entry) return;
    
    memset(node, 0, sizeof(FileNode));
    
    /* NOTE: Name handling is now done by LFN parser in find structure, 
       but for 8.3 fallback we still keep this logic optionally, or expect caller to set name. 
       This function will populate 8.3 name if node->name is empty? 
       Actually, let's keep it doing 8.3, and caller overwrites with LFN if present. */
    
    char name[13] = {0};
    int name_len = 0;
    
    for (int i = 0; i < 8; i++) {
        if (entry->name[i] != ' ') name[name_len++] = entry->name[i];
    }
    
    if (entry->ext[0] != ' ') {
        name[name_len++] = '.';
        for (int i = 0; i < 3; i++) {
            if (entry->ext[i] != ' ') name[name_len++] = entry->ext[i];
        }
    }
    
    name[name_len] = '\0';
    
    for (int i = 0; i < name_len; i++) {
        name[i] = tolower((unsigned char)name[i]);
    }
    
    strcpy(node->name, name);
    
    node->size = entry->file_size;
    node->attributes.directory = (entry->attributes & FAT_ATTR_DIRECTORY) != 0;
    node->type = (entry->attributes & FAT_ATTR_DIRECTORY) ? FILE_TYPE_DIRECTORY : FILE_TYPE_REGULAR;
    
    node->first_cluster = entry->first_cluster_low;
    if (fat_driver_get_fat_type(driver) == FAT_TYPE_32) {
        node->first_cluster |= ((uint32_t)entry->first_cluster_high << 16);
    }
    
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

uint32_t fat_driver_get_fat_entry(FATDriver* driver, uint32_t cluster) {
    if (!driver || !driver->fat_cache) return 0;
    
    FatType fat_type = fat_driver_get_fat_type(driver);
    uint32_t fat_offset = 0;
    uint32_t sector_num = 0;
    uint32_t offset_in_sector = 0;
    uint32_t sector_size = hal_get_sector_size(driver->hal);
    
    if (fat_type == FAT_TYPE_12) {
        fat_offset = cluster + (cluster / 2);
    } else if (fat_type == FAT_TYPE_16) {
        fat_offset = cluster * 2;
    } else if (fat_type == FAT_TYPE_32) {
        fat_offset = cluster * 4;
    }
    
    sector_num = driver->first_fat_sector + (fat_offset / sector_size);
    offset_in_sector = fat_offset % sector_size;
    
    if (driver->fat_cache_sector != sector_num) {
        if ((uint32_t)hal_read_sector(driver->hal, sector_num, driver->fat_cache) != sector_size) {
            return 0;
        }
        driver->fat_cache_sector = sector_num;
    }
    
    uint8_t* cache = driver->fat_cache;
    
    if (fat_type == FAT_TYPE_12) {
        uint16_t fat_entry;
        
        if (offset_in_sector == sector_size - 1) {
            uint8_t byte1 = cache[offset_in_sector];
            sector_num++;
            
            if (driver->fat_cache_sector != sector_num) {
                if ((uint32_t)hal_read_sector(driver->hal, sector_num, driver->fat_cache) != sector_size) {
                    return 0;
                }
                driver->fat_cache_sector = sector_num;
            }
            uint8_t byte2 = driver->fat_cache[0];
            fat_entry = byte1 | ((uint16_t)byte2 << 8);
        } else {
            fat_entry = *(uint16_t*)(cache + offset_in_sector);
        }
        
        if (cluster & 0x1) {
            return fat_entry >> 4;
        } else {
            return fat_entry & 0x0FFF;
        }
    } else if (fat_type == FAT_TYPE_16) {
        return *(uint16_t*)(cache + offset_in_sector);
    } else if (fat_type == FAT_TYPE_32) {
        return *(uint32_t*)(cache + offset_in_sector) & 0x0FFFFFFF;
    }
    
    return 0;
}

uint32_t fat_driver_get_next_cluster(FATDriver* driver, uint32_t cluster) {
    if (!driver) return 0;
    return fat_driver_get_fat_entry(driver, cluster);
}

/**
 * Check if a cluster value indicates End-of-Chain.
 */
int fat_driver_is_eoc(FATDriver* driver, uint32_t cluster) {
    if (!driver) return 1; /* Treat invalid as EOC */
    
    FatType fat_type = fat_driver_get_fat_type(driver);
    
    if (fat_type == FAT_TYPE_12) {
        return cluster >= 0x0FF8;
    } else if (fat_type == FAT_TYPE_16) {
        return cluster >= 0xFFF8;
    } else if (fat_type == FAT_TYPE_32) {
        return cluster >= 0x0FFFFFF8;
    }
    
    return 1;
}

/**
 * List all entries in a directory (populates dir->children).
 * Call this before iterating dir->children.
 */
int fat_driver_list_directory(FATDriver* driver, FileNode* dir) {
    if (!driver || !dir || dir->type != FILE_TYPE_DIRECTORY) return -1;
    
    /* Free existing children to avoid duplicates */
    FileNode* child = dir->children;
    while (child) {
        FileNode* next = child->next;
        fat_driver_free_file_node(child);
        child = next;
    }
    dir->children = NULL;
    
    uint32_t sector_size = hal_get_sector_size(driver->hal);
    uint8_t* buffer = malloc(sector_size);
    if (!buffer) return -1;
    
    uint32_t current_cluster = dir->first_cluster;
    
    /* LFN buffer */
    char lfn_buffer[256] = {0};
    int has_lfn = 0;
    
    /* Handling ROOT_DIR for FAT12/16 (sectors, not clusters) */
    int is_root_16 = (dir == driver->root_directory && fat_driver_get_fat_type(driver) != FAT_TYPE_32);
    uint32_t root_sectors_count = is_root_16 ? driver->root_dir_sectors : 0;
    uint32_t root_current_sector_idx = 0;
    
    while (1) {
        /* Condition loop: cluster based or fixed sector based */
        if (is_root_16) {
            if (root_current_sector_idx >= root_sectors_count) break;
        } else {
            if (current_cluster == 0 || fat_driver_is_eoc(driver, current_cluster)) break;
        }
        
        uint32_t first_sector = 0;
        uint32_t sector_count = 0;
        
        if (is_root_16) {
            first_sector = driver->first_root_dir_sector + root_current_sector_idx;
            sector_count = 1;
            root_current_sector_idx++;
        } else {
            first_sector = fat_driver_cluster_to_sector(driver, current_cluster);
            sector_count = driver->boot_sector.sectors_per_cluster;
        }
        
        for (uint32_t s = 0; s < sector_count; s++) {
            int read_result = hal_read_sector(driver->hal, first_sector + s, buffer);
            if (read_result < 0 || (uint32_t)read_result != sector_size) {
                free(buffer);
                return -1;
            }
            
            for (uint32_t off = 0; off < sector_size; off += 32) {
                FATDirEntry* entry = (FATDirEntry*)(buffer + off);
                
                if (entry->name[0] == 0x00) { /* End of dir */
                    free(buffer);
                    return 0;
                }
                if (entry->name[0] == 0xE5) { /* Deleted */
                    has_lfn = 0;
                    continue;
                }
                
                if (entry->attributes == FAT_ATTR_LFN) {
                    LFNEntry* lfn = (LFNEntry*)entry;
                    if (lfn->order & 0x40) { /* Last LFN entry (first in sequence) */
                        memset(lfn_buffer, 0, sizeof(lfn_buffer));
                        has_lfn = 1;
                    }
                    
                    if (has_lfn) {
                        int index = ((lfn->order & 0x3F) - 1) * 13;
                        if (index >= 0 && index < 255) {
                            int p = 0;
                            for(int i=0; i<5; i++) if (index+p < 255) lfn_buffer[index + p++] = (char)lfn->name1[i];
                            for(int i=0; i<6; i++) if (index+p < 255) lfn_buffer[index + p++] = (char)lfn->name2[i];
                            for(int i=0; i<2; i++) if (index+p < 255) lfn_buffer[index + p++] = (char)lfn->name3[i];
                        }
                    }
                    continue;
                }
                
                /* Skip . and .. entries */
                if ((entry->name[0] == '.' && entry->name[1] == ' ') ||
                    (entry->name[0] == '.' && entry->name[1] == '.' && entry->name[2] == ' ')) {
                    has_lfn = 0;
                    continue;
                }
                
                /* Regular entry */
                FileNode* node = malloc(sizeof(FileNode));
                if (!node) {
                    free(buffer);
                    return -1;
                }
                fat_driver_fill_file_node(driver, node, entry);
                
                /* Apply LFN if present */
                if (has_lfn) {
                    strncpy(node->name, lfn_buffer, FILE_NAME_MAX);
                    node->name[FILE_NAME_MAX] = '\0';
                    has_lfn = 0;
                }
                
                /* Add to children list */
                node->parent = dir;
                node->next = dir->children;
                dir->children = node;
            }
        }
        
        if (!is_root_16) {
            current_cluster = fat_driver_get_next_cluster(driver, current_cluster);
        }
    }
    
    free(buffer);
    return 0;
}

/**
 * Helper to find a file in a specific directory node by scanning its sectors.
 * Supports LFN.
 */
static FileNode* fat_driver_find_in_directory(FATDriver* driver, FileNode* dir, const char* name) {
    if (!driver || !dir || !name) return NULL;
    
    uint32_t sector_size = hal_get_sector_size(driver->hal);
    uint8_t* buffer = malloc(sector_size);
    if (!buffer) return NULL;
    
    uint32_t current_cluster = dir->first_cluster;
    
    // LFN buffer
    char lfn_buffer[256] = {0};
    int has_lfn = 0;
    
    // Handling ROOT_DIR for FAT12/16 (sectors, not clusters)
    int is_root_16 = (dir == driver->root_directory && fat_driver_get_fat_type(driver) != FAT_TYPE_32);
    uint32_t root_sectors_count = is_root_16 ? driver->root_dir_sectors : 0;
    uint32_t root_current_sector_idx = 0;
    
    while (1) {
        // Condition loop: cluster based or fixed sector based
        if (is_root_16) {
            if (root_current_sector_idx >= root_sectors_count) break;
        } else {
            if (current_cluster == 0 || current_cluster >= 0x0FFFFFF8) break; // End of chain
        }
        
        uint32_t first_sector = 0;
        uint32_t sector_count = 0;
        
        if (is_root_16) {
            first_sector = driver->first_root_dir_sector + root_current_sector_idx;
            sector_count = 1; // Read 1 sector at a time in loop
            root_current_sector_idx++;
        } else {
            first_sector = fat_driver_cluster_to_sector(driver, current_cluster);
            sector_count = driver->boot_sector.sectors_per_cluster;
        }
        
        for (uint32_t s = 0; s < sector_count; s++) {
            int read_result = hal_read_sector(driver->hal, first_sector + s, buffer);
            if (read_result < 0 || (uint32_t)read_result != sector_size) {
                free(buffer);
                return NULL;
            }
            
            for (uint32_t off = 0; off < sector_size; off += 32) {
                FATDirEntry* entry = (FATDirEntry*)(buffer + off);
                
                if (entry->name[0] == 0x00) { // End of dir
                    free(buffer);
                    return NULL;
                }
                if (entry->name[0] == 0xE5) { // Deleted
                    has_lfn = 0;
                    continue;
                }
                
                if (entry->attributes == FAT_ATTR_LFN) {
                    LFNEntry* lfn = (LFNEntry*)entry;
                    if (lfn->order & 0x40) { // Last LFN entry (first in sequence)
                        memset(lfn_buffer, 0, sizeof(lfn_buffer));
                        has_lfn = 1;
                        /* TODO: Implement checksum verification if needed */
                    }
                    
                    if (has_lfn) {
                        int index = ((lfn->order & 0x3F) - 1) * 13;
                        if (index >= 0 && index < 255) {
                            // Extract unicode chars, simple cast to char for now
                            int p = 0;
                            for(int i=0; i<5; i++) if (index+p < 255) lfn_buffer[index + p++] = (char)lfn->name1[i];
                            for(int i=0; i<6; i++) if (index+p < 255) lfn_buffer[index + p++] = (char)lfn->name2[i];
                            for(int i=0; i<2; i++) if (index+p < 255) lfn_buffer[index + p++] = (char)lfn->name3[i];
                        }
                    }
                    continue;
                }
                
                // Regular entry
                FileNode* node = malloc(sizeof(FileNode));
                fat_driver_fill_file_node(driver, node, entry);
                
                // Check LFN match
                if (has_lfn) {
                    // TODO: verify checksum
                     strcpy(node->name, lfn_buffer);
                     has_lfn = 0; // Reset
                }
                
                // Compare name
                if (strcasecmp(node->name, name) == 0) {
                    free(buffer);
                    return node;
                }
                
                free(node);
                has_lfn = 0;
            }
        }
        
        if (!is_root_16) {
            current_cluster = fat_driver_get_next_cluster(driver, current_cluster);
        }
    }
    
    free(buffer);
    return NULL;
}
