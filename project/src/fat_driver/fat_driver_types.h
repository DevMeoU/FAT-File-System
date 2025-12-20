/**
 * @file fat_driver_types.h
 * @author Le Duc Son (sonld@hselab.com)
 * @date 2020-11-24
 * @brief FAT Driver data types
 * @details This file contains the data types used in FAT Driver module
 */

#ifndef FAT_DRIVER_TYPES_H
#define FAT_DRIVER_TYPES_H

#include <stdint.h>
#include "../common/common_types.h"

/**
 * Boot Sector structure
 */
typedef struct {
    uint16_t bytes_per_sector;      /**< Offset 11-12: Number of bytes per sector */
    uint8_t sectors_per_cluster;    /**< Offset 13: Number of sectors per cluster */
    uint16_t reserved_sectors;      /**< Offset 14-15: Number of reserved sectors */
    uint8_t number_of_fats;         /**< Offset 16: Number of FATs */
    uint16_t root_entry_count;      /**< Offset 17-18: Number of entries in root directory (FAT12/16) */
    uint16_t total_sectors_16;      /**< Offset 19-20: Total number of sectors (16-bit) */
    uint8_t media_type;             /**< Offset 21: Media type */
    uint16_t fat_size_16;           /**< Offset 22-23: Number of sectors per FAT (FAT12/16) */
    uint16_t sectors_per_track;     /**< Offset 24-25: Number of sectors per track */
    uint16_t number_of_heads;       /**< Offset 26-27: Number of heads */
    uint32_t hidden_sectors;        /**< Offset 28-31: Number of hidden sectors */
    uint32_t total_sectors_32;      /**< Offset 32-35: Total number of sectors (32-bit) */
    
    /**
     * Additional fields for FAT32
     */
    uint32_t fat_size_32;           /**< Offset 36-39: Number of sectors per FAT (FAT32) */
    uint16_t extended_flags;        /**< Offset 40-41: Extended flags */
    uint16_t fs_version;            /**< Offset 42-43: File system version */
    uint32_t root_cluster;          /**< Offset 44-47: Cluster number of root directory */
    uint16_t fs_info;               /**< Offset 48-49: Sector number of FS_INFO */
    uint16_t backup_boot_sector;    /**< Offset 50-51: Sector number of backup boot sector */
    uint8_t reserved[12];           /**< Offset 52-63: Reserved */
    uint8_t drive_number;           /**< Offset 64: Drive number */
    uint8_t reserved1;              /**< Offset 65: Reserved */
    uint8_t boot_signature;         /**< Offset 66: Boot signature */
    uint32_t volume_id;             /**< Offset 67-70: Volume ID */
    char volume_label[11];          /**< Offset 71-81: Volume label */
    char fs_type[8];                /**< Offset 82-89: File system type */
} BootSector;

/**
 * Directory Entry structure
 */
typedef struct __attribute__((packed)) {
    char name[8];                   /**< Name (8 characters) */
    char extension[3];              /**< Extension (3 characters) */
    uint8_t attributes;             /**< Attributes */
    uint8_t reserved;               /**< Reserved */
    uint8_t creation_time_tenth;    /**< Tenths of seconds of creation time */
    uint16_t creation_time;         /**< Creation time */
    uint16_t creation_date;         /**< Creation date */
    uint16_t last_access_date;      /**< Last access date */
    uint16_t first_cluster_high;    /**< High part of first cluster (FAT32) */
    uint16_t last_modification_time; /**< Last modification time */
    uint16_t last_modification_date; /**< Last modification date */
    uint16_t first_cluster_low;     /**< Low part of first cluster */
    uint32_t file_size;             /**< File size */
} DirectoryEntry;

/**
 * LFN (Long File Name) Entry structure
 */
typedef struct __attribute__((packed)) {
    uint8_t order;                  /**< Order of entry */
    uint16_t name1[5];              /**< First 5 characters (Unicode) */
    uint8_t attribute;              /**< Attribute (always 0x0F for LFN) */
    uint8_t type;                   /**< Type (0 for LFN) */
    uint8_t checksum;               /**< Checksum */
    uint16_t name2[6];              /**< Next 6 characters (Unicode) */
    uint16_t first_cluster;         /**< Always 0 for LFN */
    uint16_t name3[2];              /**< Last 2 characters (Unicode) */
} LFNEntry;

/**
 * File/Directory structure
 */
typedef struct FileNode {
    char name[FILE_NAME_MAX + 1];   /**< Name */
    FileType type;                  /**< Type (file/directory) */
    FileAttributes attributes;      /**< Attributes */
    uint32_t size;                  /**< Size */
    uint32_t first_cluster;         /**< First cluster */
    uint32_t first_sector;          /**< First sector */
    DateTime created_time;          /**< Creation time */
    DateTime modified_time;         /**< Last modification time */
    struct FileNode* parent;        /**< Parent directory */
    struct FileNode* children;      /**< List of children (if directory) */
    struct FileNode* next;          /**< Next node in same directory */
} FileNode;

/**
 * FAT Driver structure
 */
typedef struct {
    HAL* hal;                       /**< Pointer to HAL */
    FileSystemConfig config;        /**< File system configuration */
    BootSector boot_sector;         /**< Boot sector */
    uint8_t* fat_cache;             /**< FAT table sector cache */
    uint32_t fat_cache_sector;      /**< Current sector loaded in FAT cache */
                                    /**< Note: Valid only if fat_cache != NULL */
    uint32_t first_fat_sector;      /**< First sector of FAT */
    uint32_t first_data_sector;     /**< First sector of data area */
    uint32_t root_dir_sectors;      /**< Number of sectors of root directory (FAT12/16) */
    uint32_t first_root_dir_sector; /**< First sector of root directory (FAT12/16) */
    uint32_t data_sectors;          /**< Number of sectors of data area */
    uint32_t total_clusters;        /**< Total number of clusters */
    FileNode* root_directory;       /**< Root directory */
    FileNode* current_directory;    /**< Current directory */
    void* cache;                    /**< Cache */
    uint32_t cache_size;            /**< Cache size */
} FATDriver;

#endif // FAT_DRIVER_TYPES_H

