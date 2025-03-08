#ifndef __FAT_DRIVER_TYPES_H
#define __FAT_DRIVER_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/* FAT Types */
#define FAT_TYPE_12      12
#define FAT_TYPE_16      16
#define FAT_TYPE_32      32

/* FAT Constants */
#define FAT_SECTOR_SIZE  512
#define FAT_SIGNATURE_AA55 0xAA55

/* File Attributes */
#define FAT_ATTR_READ_ONLY  0x01
#define FAT_ATTR_HIDDEN     0x02
#define FAT_ATTR_SYSTEM     0x04
#define FAT_ATTR_VOLUME_ID  0x08
#define FAT_ATTR_DIRECTORY  0x10
#define FAT_ATTR_ARCHIVE    0x20
#define FAT_ATTR_LONG_NAME   (FAT_ATTR_READ_ONLY | FAT_ATTR_HIDDEN | \
                             FAT_ATTR_SYSTEM | FAT_ATTR_VOLUME_ID)

/* FAT Entry Values */
#define FAT12_MASK       0x0FFF
#define FAT16_MASK       0xFFFF
#define FAT32_MASK       0x0FFFFFFF
#define FAT12_EOC        0x0FF8
#define FAT16_EOC        0xFFF8
#define FAT32_EOC        0x0FFFFFF8
#define FAT_FREE_CLUSTER 0x00000000

/* File Access Modes */
#define FAT_MODE_READ       0x01
#define FAT_MODE_WRITE      0x02
#define FAT_MODE_CREATE     0x04
#define FAT_MODE_APPEND     0x08
#define FAT_MODE_TRUNCATE   0x10

/* Forward declarations */
typedef struct fat_dir_entry fat_dir_entry_t;
typedef struct fat_file_info fat_file_info_t;
typedef struct fat_file fat_file_t;
typedef struct fat_config fat_config_t;
typedef struct fat_boot_sector fat_boot_sector_t;

/* FAT Boot Sector */
struct __attribute__((packed)) fat_boot_sector {
    uint8_t  jump_boot[3];
    uint8_t  oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  num_fats;
    uint16_t root_entries;
    uint16_t total_sectors_16;
    uint8_t  media_type;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    union {
        struct {
            uint8_t  drive_number;
            uint8_t  reserved1;
            uint8_t  boot_signature;
            uint32_t volume_id;
            uint8_t  volume_label[11];
            uint8_t  fs_type[8];
        } fat16;
        struct {
            uint32_t fat_size_32;
            uint16_t ext_flags;
            uint16_t fs_version;
            uint32_t root_cluster;
            uint16_t fs_info;
            uint16_t backup_boot;
            uint8_t  reserved[12];
            uint8_t  drive_number;
            uint8_t  reserved1;
            uint8_t  boot_signature;
            uint32_t volume_id;
            uint8_t  volume_label[11];
            uint8_t  fs_type[8];
        } fat32;
    };
    uint16_t signature;
};

/* FAT Directory Entry */
struct __attribute__((packed)) fat_dir_entry {
    uint8_t  name[11];           /* 8.3 filename */
    uint8_t  attributes;         /* File attributes */
    uint8_t  reserved;          /* Reserved for Windows NT */
    uint8_t  creation_time_ms;  /* Creation time, milliseconds */
    uint16_t creation_time;     /* Creation time */
    uint16_t creation_date;     /* Creation date */
    uint16_t last_access_date;  /* Last access date */
    uint16_t first_cluster_hi;  /* High word of first cluster number */
    uint16_t last_write_time;   /* Last write time */
    uint16_t last_write_date;   /* Last write date */
    uint16_t first_cluster_lo;  /* Low word of first cluster number */
    uint32_t file_size;         /* File size in bytes */
};

/* File Information Structure */
struct fat_file_info {
    char     name[256];
    uint32_t size;
    uint8_t  attributes;
    uint32_t cluster;
    uint16_t date;
    uint16_t time;
};

/* File Handle Structure */
struct fat_file {
    fat_file_info_t info;
    uint32_t position;
    uint32_t cluster;
    uint32_t sector;
    uint32_t offset;
    uint8_t  mode;
    bool     modified;
};

/* FAT Configuration Structure */
struct fat_config {
    uint8_t  fat_type;          /* FAT type (12/16/32) */
    uint32_t total_sectors;     /* Total number of sectors */
    uint32_t bytes_per_sector;  /* Number of bytes per sector */
    uint32_t sectors_per_cluster; /* Number of sectors per cluster */
    uint32_t reserved_sectors;  /* Number of reserved sectors */
    uint32_t number_of_fats;    /* Number of FAT copies */
    uint32_t root_entries;      /* Maximum number of root directory entries */
    uint32_t total_clusters;    /* Total number of clusters */
    bool     use_cache;         /* Whether to use sector caching */
};

#endif /* __FAT_DRIVER_TYPES_H */ 