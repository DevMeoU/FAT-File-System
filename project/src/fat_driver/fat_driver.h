/**
 * @file fat_driver.h
 * @author Le Duc Son
 * @date 2017-11-15
 */

#ifndef FAT_DRIVER_H
#define FAT_DRIVER_H

#include "../common/common_types.h"
#include "../hal/hal.h"
#include "fat_driver_types.h"

/**
 * Initialize FAT Driver
 * @param driver Pointer to FATDriver structure
 * @param config File system configuration
 * @return 0 if successful, -1 if failed
 */
int fat_driver_init(FATDriver* driver, const FileSystemConfig config);

/**
 * Deinitialize FAT Driver
 * @param driver Pointer to FATDriver structure
 * @return 0 if successful, -1 if failed
 */
int fat_driver_deinit(FATDriver* driver);

/**
 * Mount the file system
 * @param driver Pointer to FATDriver structure
 * @return 0 if successful, -1 if failed
 */
int fat_driver_mount(FATDriver* driver);

/**
 * Unmount the file system
 * @param driver Pointer to FATDriver structure
 */
void fat_driver_unmount(FATDriver* driver);

/**
 * Get the root directory
 * @param driver Pointer to FATDriver structure
 * @return Pointer to the root directory
 */
FileNode* fat_driver_get_root_directory(FATDriver* driver);

/**
 * Get the current directory
 * @param driver Pointer to FATDriver structure
 * @return Pointer to the current directory
 */
FileNode* fat_driver_get_current_directory(FATDriver* driver);

/**
 * Set the current directory
 * @param driver Pointer to FATDriver structure
 * @param directory Pointer to the directory to set
 * @return 0 if successful, -1 if failed
 */
int fat_driver_set_current_directory(FATDriver* driver, FileNode* directory);

/**
 * Find a path
 * @param driver Pointer to FATDriver structure
 * @param path Path to find
 * @return Pointer to the node if found, NULL if not found
 */
FileNode* fat_driver_find_path(FATDriver* driver, const char* path);

/**
 * Recursively find a path
 * @param current Pointer to the current node
 * @param path Path to find
 * @return Pointer to the node if found, NULL if not found
 */
FileNode* fat_driver_find_path_recursive(FileNode* current, const char* path);

/**
 * Read file content
 * @param driver Pointer to FATDriver structure
 * @param file Pointer to the file to read
 * @param buffer Buffer to store the read data
 * @param size Size to read
 * @return Number of bytes read if successful, -1 if failed
 */
int fat_driver_read_file(FATDriver* driver, FileNode* file, void* buffer, uint32_t size);

/**
 * Write file content
 * @param driver Pointer to FATDriver structure
 * @param file Pointer to the file to write
 * @param buffer Buffer with the data to write
 * @param size Size to write
 * @return Number of bytes written if successful, -1 if failed
 */
int fat_driver_write_file(FATDriver* driver, FileNode* file, const void* buffer, uint32_t size);

/**
 * Get FAT type
 * @param driver Pointer to FATDriver structure
 * @return FAT type (FAT12, FAT16, FAT32)
 */
FatType fat_driver_get_fat_type(FATDriver* driver);

/**
 * Get file system information
 * @param driver Pointer to FATDriver structure
 * @param total_size Pointer to store total size
 * @param free_size Pointer to store free size
 * @return 0 if successful, -1 if failed
 */
int fat_driver_get_filesystem_info(FATDriver* driver, uint64_t* total_size, uint64_t* free_size);

/**
 * Convert cluster to sector
 * @param driver Pointer to FATDriver structure
 * @param cluster Cluster number
 * @return Sector number
 */
uint32_t fat_driver_cluster_to_sector(FATDriver* driver, uint32_t cluster);

/**
 * Check if cluster is End-of-Chain
 * @param driver Pointer to FATDriver structure
 * @param cluster Cluster number to check
 * @return 1 if EOC, 0 otherwise
 */
int fat_driver_is_eoc(FATDriver* driver, uint32_t cluster);

/**
 * List all entries in a directory (populates dir->children)
 * @param driver Pointer to FATDriver structure
 * @param dir Pointer to directory node
 * @return 0 if successful, -1 if failed
 */
int fat_driver_list_directory(FATDriver* driver, FileNode* dir);

/**
 * Free file node
 * @param node Pointer to the node to free
 */
void fat_driver_free_file_node(FileNode* node);

#endif /* FAT_DRIVER_H */

