/*
* FAT Driver Header
* Author: Ducson9112k
*/

#ifndef FAT_DRIVER_H
#define FAT_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <ctype.h>
#include "linkedlist.h"
#include "print_color.h"

/*===============================*
*         Macro Definitions
*===============================*/

/* FAT driver status */
#define FAT_DRIVER_ON  1U
#define FAT_DRIVER_OFF 0U

/* Debug mode */
#define FAT_DRIVER_DEBUG_MODE FAT_DRIVER_OFF

/* Bytes per sector */
#define FAT_DRIVER_BYTES_PER_SECTOR 512U

/* Boot Sector related */
#define FAT_DRIVER_NUMBER_OF_BOOT_SECTORS 1U
#define FAT_DRIVER_MAX_CLUSTER_OF_FATFS_12 4085U
#define FAT_DRIVER_MAX_CLUSTER_OF_FATFS_16 65525U

/* Directory Entry definitions */
#define DIR_NUMBER_OF_BYTES_PER_ENTRY 32U
#define DIR_NAME_LEN            8U
#define DIR_EXTENTION_LEN       3U

/* File attributes */
#define FAT_DRIVER_ATTR_READ_ONLY      0x01U
#define FAT_DRIVER_ATTR_HIDDEN_FILE    0x02U
#define FAT_DRIVER_ATTR_SYSTEM_FILE    0x04U
#define FAT_DRIVER_ATTR_VOLUME_LABEL   0x08U
#define FAT_DRIVER_ATTR_LONG_FILE_NAME 0x0FU
#define FAT_DRIVER_ATTR_DIRECTORY      0x10U
#define FAT_DRIVER_ATTR_ARCHIVE_FLAG   0x20U

/* Filename definitions for SFN */
#define FAT_DRIVER_SFN_NAME_PART    DIR_NAME_LEN
#define FAT_DRIVER_SFN_EXT_LENGTH   DIR_EXTENTION_LEN
#define FAT_DRIVER_SFN_BUFFER_SIZE  (FAT_DRIVER_SFN_NAME_PART + FAT_DRIVER_SFN_EXT_LENGTH + 2)

/* Special filename markers */
#define FAT_DRIVER_FILENAME_NEVER_USED   0x00U
#define FAT_DRIVER_FILENAME_DELETED_1    0xE5U
#define FAT_DRIVER_FILENAME_DELETED_2    0x05U
#define FAT_DRIVER_FILENAME_SPECIAL_ENTRY 0x2EU

/* End Of Cluster for FAT12 */
#define FAT_DRIVER_12_LAST_CLUSTER_START 0xFF8U
#define FAT_DRIVER_12_LAST_CLUSTER_END   0xFFFU
#define FAT_DRIVER_12_BAD_CLUSTER        0xFF7U
#define FAT_DRIVER_12_EOC                FAT_DRIVER_12_LAST_CLUSTER_END

/* Bit Mask of hours, minutes, seconds*/
#define FAT_DRIVER_BIT_MASK_OF_HOURS 0XF800U
#define FAT_DRIVER_BIT_MASK_OF_MINUTES 0x7E0U
#define FAT_DRIVER_BIT_MASK_OF_SECONDS 0x1FU

/* Shift bit mask Hour, Minute, Second */
#define FAT_DRIVER_SHIFT_HOURS 0xBU
#define FAT_DRIVER_SHIFT_MINUTES 0x5U
#define FAT_DRIVER_SHIFT_SECONDS 0x00U

/* Calculate hours, minutes, seconds */
#define FAT_DRIVER_EXTRACT_HOURS(x) (((FAT_DRIVER_BIT_MASK_OF_HOURS) & (x)) >> (FAT_DRIVER_SHIFT_HOURS))
#define FAT_DRIVER_EXTRACT_MINUTES(x) (((FAT_DRIVER_BIT_MASK_OF_MINUTES) & (x)) >> (FAT_DRIVER_SHIFT_MINUTES))
#define FAT_DRIVER_EXTRACT_SECONDS(x) ((FAT_DRIVER_BIT_MASK_OF_SECONDS) & (x) >> (FAT_DRIVER_SHIFT_SECONDS))

/* Bit mask of year, month, day */
#define FAT_DRIVER_BIT_MASK_OF_YEAR 0xFE00U
#define FAT_DRIVER_BIT_MASK_OF_MONTH 0x1E0U
#define FAT_DRIVER_BIT_MASK_OF_DAY 0x1FU

/* Shift bit mask year, month, day */
#define FAT_DRIVER_SHIFT_YEAR 9U
#define FAT_DRIVER_SHIFT_MONTH 5U
#define FAT_DRIVER_SHIFT_DAY 0x00U

#define BIN_YEAR_OFFSET_FROM 1980U
/* Calculate year, month, day */
#define FAT_DRIVER_EXTRACT_YEAR(y) (((FAT_DRIVER_BIT_MASK_OF_YEAR) & (y)) >> (FAT_DRIVER_SHIFT_YEAR))
#define FAT_DRIVER_EXTRACT_MONTH(y) (((FAT_DRIVER_BIT_MASK_OF_MONTH) & (y)) >> (FAT_DRIVER_SHIFT_MONTH))
#define FAT_DRIVER_EXTRACT_DAY(y) (((FAT_DRIVER_BIT_MASK_OF_DAY) & (y)) >> (FAT_DRIVER_SHIFT_DAY))

/*===============================*
*         Type Definitions
*===============================*/
/* Nếu chưa được định nghĩa, định nghĩa BIN_YEAR_OFFSET_FROM */
#ifndef BIN_YEAR_OFFSET_FROM
#define BIN_YEAR_OFFSET_FROM 1980U
#endif

/* Nếu chưa được định nghĩa, tính số entry trên mỗi sector */
#ifndef FAT_DRIVER_ENTRYS_OF_SECTOR
#define FAT_DRIVER_ENTRYS_OF_SECTOR (FAT_DRIVER_BYTES_PER_SECTOR / DIR_NUMBER_OF_BYTES_PER_ENTRY)
#endif

/*---------------------------------------------------------------------
 *          LOCAL STRUCTURES FOR BOOT SECTOR FORMATS
 *---------------------------------------------------------------------*/
/* Cấu trúc Boot Sector cho FAT12/16 (định dạng 126 bytes) */
typedef struct __attribute__((__packed__)) _FATFS_BootFormat126 {
    uint8_t BS_JmpBoot[3];               /* Jump instruction */
    uint8_t BS_OEMName[8];               /* OEM Name */
    uint8_t BPB_BytsPerSec[2];           /* Bytes per sector */
    uint8_t BPB_SecPerClus[1];           /* Sectors per cluster */
    uint8_t BPB_RsvdSecCnt[2];           /* Reserved sectors count */
    uint8_t BPB_NumFATs[1];              /* Number of FATs */
    uint8_t BPB_RootEntCnt[2];           /* Root entries count */
    uint8_t BPB_TotSec16[2];             /* Total sectors (16-bit) */
    uint8_t BPB_Media[1];                /* Media descriptor */
    uint8_t BPB_FATSz16[2];              /* FAT size (16-bit) */
    uint8_t BPB_SecPerTrk[2];            /* Sectors per track */
    uint8_t BPB_NumHeads[2];             /* Number of heads */
    uint8_t BPB_HiddSec[4];              /* Hidden sectors */
    uint8_t BPB_TotSec32[4];             /* Total sectors (32-bit) */
} FATFS_BootFormat126_t;

/* Cấu trúc Boot Sector cho FAT32 */
typedef struct __attribute__((__packed__)) _FATFS_BootFormat32 {
    uint8_t BS_JmpBoot[3];               /* Jump instruction */
    uint8_t BS_OEMName[8];               /* OEM Name */
    uint8_t BPB_BytsPerSec[2];           /* Bytes per sector */
    uint8_t BPB_SecPerClus[1];           /* Sectors per cluster */
    uint8_t BPB_RsvdSecCnt[2];           /* Reserved sectors count */
    uint8_t BPB_NumFATs[1];              /* Number of FATs */
    uint8_t BPB_RootEntCnt[2];           /* Root entries count */
    uint8_t BPB_TotSec16[2];             /* Total sectors (16-bit) */
    uint8_t BPB_Media[1];                /* Media descriptor */
    uint8_t BPB_FATSz16[2];              /* FAT size (16-bit); zero for FAT32 */
    uint8_t BPB_SecPerTrk[2];            /* Sectors per track */
    uint8_t BPB_NumHeads[2];             /* Number of heads */
    uint8_t BPB_HiddSec[4];              /* Hidden sectors */
    uint8_t BPB_TotSec32[4];             /* Total sectors (32-bit) */
    uint8_t BPB_FATSz32[4];              /* FAT size (32-bit) */
    /* Các trường khác của FAT32 có thể được thêm nếu cần */
} FATFS_BootFormat32_t;

/* FAT Boot Data Structure */
typedef struct {
    uint32_t FirstRootClus;
    uint32_t FirstDataClus;
    uint32_t RootDirSector;
    uint32_t BytePerSec;
    uint32_t SectorPerClus;
    uint32_t RsvdSecCnt;
    uint32_t NumFats;
    uint32_t RootEntCnt;
    uint32_t TotSec;
    uint32_t FatSz;
    uint32_t DataSec;
} FATFS_BootData_t;

/* FAT File Information Structure */
typedef struct {
    char name[12];
    uint8_t attributes;
    uint16_t creationTime;
    uint16_t creationDate;
    uint16_t lastAccessDate;
    uint16_t lastWriteTime;
    uint16_t lastWriteDate;
    uint32_t firstCluster;
    uint32_t fileSize;
    struct {
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
        uint8_t day;
        uint8_t month;
        uint16_t year;
    } crtTime, wrtTime;
} FAT_DRIVER_FileInfo;

/* FAT Filesystem Types */
typedef enum _FATFS_TYPES {
    ERROR = -1,
    FAT12 = 0U,
    FAT16 = 1U,
    FAT32 = 2U
} FATFS_FatTypes_t;

/* Structure for directory entry format (SFN) */
typedef struct __attribute__((__packed__)) _ENTRY_FORMAT {
    uint8_t DIR_Name[DIR_NAME_LEN];
    uint8_t DIR_Ext[DIR_EXTENTION_LEN];
    uint8_t DIR_Attr[1];
    uint8_t DIR_NTRes[1];
    uint8_t DIR_CrtTimeTenth[1];
    uint8_t DIR_CrtTime[2];
    uint8_t DIR_CrtDate[2];
    uint8_t DIR_LstAccDate[2];
    uint8_t DIR_FstClusHI[2];
    uint8_t DIR_WrtTime[2];
    uint8_t DIR_WrtDate[2];
    uint8_t DIR_FstClusLO[2];
    uint8_t DIR_FileSize[4];
} FATFS_EntryFormat_t;

/* Storage data for root directory */
typedef struct fat_driver_data {
    uint32_t fistCluster;
    const char *name;
} FATFS_StorageData_t;

/*===============================*
*      Function Prototypes
*===============================*/

/**
 * @brief Initialize FAT Driver.
 *
 * @param img_path Path to the FAT image file.
 * @param file_list Pointer to the linked list to store directory tree.
 * @return 0 on success, -1 on failure.
 */
int fat_driver_init(const char *img_path, linkedlist_t *file_list);

/**
 * @brief Read boot sector.
 *
 * @return 0 on success, -1 on failure.
 */
int fat_driver_read_boot_sector(void);

/**
 * @brief List directory contents.
 *
 * @param path Path of the directory to list.
 * @param file_list Linked list to store directory entries.
 * @return 0 on success, -1 on failure.
 */
int fat_driver_list_directory(const char *path, linkedlist_t *file_list);

/**
 * @brief Read file.
 *
 * @param filename Full path of the file to read.
 * @param file_list Linked list containing the directory tree.
 * @return 0 on success, -1 on failure.
 */
int fat_driver_read_file(const char *filename, linkedlist_t *file_list);

/**
 * @brief Check if a directory exists.
 *
 * @param path Path of the directory.
 * @return 1 if exists, 0 otherwise.
 */
int fat_driver_directory_exists(const char *path);

/**
 * @brief Get the next cluster number.
 *
 * @param current_cluster Current cluster number.
 * @return Next cluster number, or -1 on error/end-of-chain.
 */
int fat_driver_get_next_cluster(uint32_t current_cluster);

#ifdef __cplusplus
}
#endif

#endif /* FAT_DRIVER_H */
