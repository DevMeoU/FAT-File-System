/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   File định nghĩa các kiểu dữ liệu cho FAT File System module.
 *********************************************************************/
#ifndef FAT_DRIVER_TYPES_H
#define FAT_DRIVER_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include "../common/common_types.h"

/* FAT File System Types */
typedef enum {
    FAT_TYPE_12 = 12,
    FAT_TYPE_16 = 16,
    FAT_TYPE_32 = 32
} fat_type_t;

/* FAT File Access Modes */
#define FAT_MODE_READ         0x01
#define FAT_MODE_WRITE        0x02
#define FAT_MODE_APPEND       0x04
#define FAT_MODE_CREATE       0x08
#define FAT_MODE_TRUNCATE     0x10

/* FAT File Attributes */
#define FAT_ATTR_READ_ONLY    0x01
#define FAT_ATTR_HIDDEN       0x02
#define FAT_ATTR_SYSTEM       0x04
#define FAT_ATTR_VOLUME_ID    0x08
#define FAT_ATTR_DIRECTORY    0x10
#define FAT_ATTR_ARCHIVE      0x20
#define FAT_ATTR_LONG_NAME    0x0F

/* FAT Directory Entry */
#define FAT_DIR_NAME_LEN      11
#define FAT_DIR_EXT_LEN       3
#define FAT_DIR_EMPTY         0x00
#define FAT_DIR_DELETED       0xE5

/* FAT File System Constants */
#define FAT_SECTOR_SIZE       512
#define FAT_CACHE_SIZE        16
#define FAT_DIR_EMPTY         0x00
#define FAT_DIR_DELETED       0xE5
#define FAT_FREE_CLUSTER      0x00000000
#define FAT_BAD_CLUSTER       0x0FFFFFF7
#define FAT_SIGNATURE_AA55    0xAA55

/* FAT Masks */
#define FAT12_MASK            0x0FFF
#define FAT16_MASK            0xFFFF
#define FAT32_MASK            0x0FFFFFFF

/* FAT End of Chain */
#define FAT12_EOC             0x0FF8
#define FAT16_EOC             0xFFF8
#define FAT32_EOC             0x0FFFFFF8

/* FAT File System Status Codes */
typedef enum {
    FAT_STATUS_SUCCESS = 0,
    FAT_STATUS_ERROR = -1,
    FAT_STATUS_INVALID = -2,
    FAT_STATUS_NOT_FOUND = -3,
    FAT_STATUS_EXISTS = -4,
    FAT_STATUS_NO_SPACE = -5,
    FAT_STATUS_READ_ONLY = -6,
    FAT_STATUS_NOT_EMPTY = -7,
    FAT_STATUS_NOT_READY = -8,
    FAT_STATUS_READ_FAILED = -9,
    FAT_STATUS_WRITE_FAILED = -10,
    FAT_STATUS_INVALID_PARAMETER = -11
} fat_status_t;

/* FAT File System Info */
typedef struct {
    uint32_t fat_start;              /* First FAT sector */
    uint32_t fat_size;               /* FAT size in sectors */
    uint32_t root_cluster;           /* Root directory cluster */
    uint32_t first_data_sector;      /* First data sector */
    uint32_t total_clusters;         /* Total number of clusters */
    uint32_t sectors_per_cluster;    /* Sectors per cluster */
    uint32_t reserved_sectors;       /* Reserved sectors */
    uint32_t root_dir_sectors;       /* Root directory sectors */
} fat_config_internal_t;

/* FAT Cache Entry */
typedef struct {
    bool valid;                      /* Entry is valid */
    bool dirty;                      /* Entry is dirty */
    uint32_t sector;                 /* Sector number */
    uint8_t data[FAT_SECTOR_SIZE];   /* Sector data */
} fat_cache_entry_t;

/* FAT File Info */
typedef struct {
    char name[13];                   /* File name */
    uint32_t size;                   /* File size */
    uint32_t cluster;                /* First cluster */
    uint16_t date;                   /* Last write date */
    uint16_t time;                   /* Last write time */
    uint8_t attributes;              /* File attributes */
} fat_file_info_t;

/* FAT File Handle */
typedef struct {
    uint8_t mode;                    /* File access mode */
    uint32_t position;               /* Current position in file */
    uint32_t cluster;                /* Current cluster */
    uint32_t sector;                 /* Current sector */
    uint32_t offset;                 /* Current offset in sector */
    bool modified;                   /* File has been modified */
    uint8_t sector_buffer[FAT_SECTOR_SIZE]; /* Buffer for sector operations */
    fat_file_info_t info;            /* File information */
} fat_file_t;

/* FAT Path Context */
typedef struct {
    char current_path[256];          /* Current directory path */
    uint32_t current_cluster;        /* Current directory cluster */
    bool is_root;                    /* Is root directory */
    char parent_path[256];           /* Parent directory path */
    uint32_t parent_cluster;         /* Parent directory cluster */
} fat_path_context_t;

/* FAT Directory Entry */
struct fat_dir_entry {
    uint8_t name[11];           /* 8.3 format */
    uint8_t attributes;         /* File attributes */
    uint8_t reserved;           /* Reserved */
    uint8_t create_time_ms;     /* Creation time, milliseconds */
    uint16_t create_time;       /* Creation time */
    uint16_t create_date;       /* Creation date */
    uint16_t last_access_date;  /* Last access date */
    uint16_t first_cluster_hi;  /* High 16 bits of first cluster */
    uint16_t last_write_time;   /* Last write time */
    uint16_t last_write_date;   /* Last write date */
    uint16_t first_cluster_lo;  /* Low 16 bits of first cluster */
    uint32_t file_size;         /* File size in bytes */
};
typedef struct fat_dir_entry fat_dir_entry_t;

/* FAT Boot Sector */
struct fat_boot_sector {
    uint8_t jump_boot[3];       /* Jump instruction */
    uint8_t oem_name[8];        /* OEM name */
    uint16_t bytes_per_sector;  /* Bytes per sector */
    uint8_t sectors_per_cluster;/* Sectors per cluster */
    uint16_t reserved_sectors;  /* Reserved sectors */
    uint8_t num_fats;          /* Number of FATs */
    uint16_t root_entries;     /* Root directory entries */
    uint16_t total_sectors_16; /* Total sectors (16-bit) */
    uint8_t media;             /* Media descriptor */
    uint16_t fat_size_16;      /* FAT size in sectors (16-bit) */
    uint16_t sectors_per_track;/* Sectors per track */
    uint16_t num_heads;        /* Number of heads */
    uint32_t hidden_sectors;   /* Hidden sectors */
    uint32_t total_sectors_32; /* Total sectors (32-bit) */
    uint32_t fat_size_32;      /* FAT size in sectors (32-bit) */
    uint16_t ext_flags;        /* Extended flags */
    uint16_t fs_version;       /* File system version */
    uint32_t root_cluster;     /* Root directory cluster */
    uint16_t fs_info;          /* File system info sector */
    uint16_t backup_boot;      /* Backup boot sector */
    uint8_t reserved[12];      /* Reserved */
    uint8_t drive_number;      /* Drive number */
    uint8_t reserved1;         /* Reserved */
    uint8_t boot_signature;    /* Boot signature */
    uint32_t volume_id;        /* Volume ID */
    uint8_t volume_label[11];  /* Volume label */
    uint8_t fs_type[8];        /* File system type */
    uint8_t signature[2];      /* Boot sector signature */
};
typedef struct fat_boot_sector fat_boot_sector_t;

/* FAT Context */
typedef struct {
    fat_config_internal_t config;     /* File system configuration */
    fat_cache_entry_t cache[FAT_CACHE_SIZE]; /* Sector cache */
    uint32_t fat_size;               /* FAT size in sectors */
    uint32_t fat_start;              /* First FAT sector */
    uint32_t root_cluster;           /* Root directory cluster */
} fat_context_t;

/* FAT Configuration */
typedef struct {
    char file_path[256];      /* File path */
    uint32_t base_addr;       /* Base address */
    uint32_t irq_num;         /* IRQ number */
    bool use_dma;             /* Use DMA flag */
    fat_type_t fat_type;      /* FAT type */
    uint32_t sectors_per_cluster; /* Sectors per cluster */
    uint32_t first_data_sector;   /* First data sector */
    uint32_t total_clusters;      /* Total number of clusters */
    uint32_t reserved_sectors;    /* Reserved sectors */
    uint32_t root_dir_sectors;    /* Root directory sectors */
} fat_config_t;

#endif /* FAT_DRIVER_TYPES_H */

/*********************************************************************
 * UUID: 3b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/ 