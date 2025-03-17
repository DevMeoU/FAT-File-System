#define _GNU_SOURCE  /* For strdup */
/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   File triển khai các hàm của module FAT Driver.
 *   Các hàm này được sử dụng bởi các module khác.
 *********************************************************************/

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../common/common_types.h"
#include "fat_driver_types.h"
#include "fat_driver_private.h"
#include "fat_driver.h"
#include "../hal/hal.h"
#include "../utilities/log/print_color.h"

/*********************************************************************
 * Private Function Prototypes
 *********************************************************************/
/* Private functions from fat_driver_private.c */
static int32_t fat_driver_init_storage(void);
static int32_t fat_driver_validate_boot_sector(void);
static int32_t fat_driver_init_fat_tables(void);
static uint32_t fat_driver_get_fat_size(const fat_boot_sector_t *boot_sector);
static int32_t fat_driver_calculate_layout(const fat_boot_sector_t *boot_sector);

/* Private functions from fat_driver.c */
static int32_t fat_driver_is_directory(const fat_dir_entry_t *entry);
static int32_t fat_driver_list_directory(const char *path);
static int32_t fat_driver_change_directory(const char *path);
static int32_t fat_driver_resolve_path(const char *path, char *resolved_path);
static int32_t fat_driver_normalize_path(const char *path, char *normalized_path);
static int32_t fat_driver_get_parent_path(const char *path, char *parent_path);
static int32_t fat_driver_init_path_context(void);
static int32_t fat_driver_update_path_context(const char *new_path, uint32_t new_cluster);

/*********************************************************************
 * Global Variables
 *********************************************************************/
static fat_driver_private_t fat_ctx;
static fat_path_context_t path_ctx;

/*********************************************************************
 * Public Function Implementations
 *********************************************************************/

int32_t fat_init(const char* img_path)
{
    int32_t status;

    /* Copy image path */
    strncpy(fat_ctx.img_path, img_path, sizeof(fat_ctx.img_path) - 1);
    fat_ctx.img_path[sizeof(fat_ctx.img_path) - 1] = '\0';

    /* Initialize driver */
    status = fat_driver_init(&fat_ctx);
    if (status != FAT_SUCCESS) {
        return status;
    }

    /* Initialize path context */
    memset(&path_ctx, 0, sizeof(path_ctx));
    strcpy(path_ctx.current_path, "/");
    path_ctx.is_root = true;
    path_ctx.current_cluster = fat_ctx.root_cluster;

    return FAT_SUCCESS;
}

int32_t fat_open(const char *path, uint8_t mode, fat_file_t *file)
{
    int32_t status;
    fat_dir_entry_t entry;

    if (!path || !file) {
        return FAT_ERROR_INVALID;
    }

    /* Find directory entry */
    status = fat_driver_find_dir_entry(&fat_ctx, path, &entry);
    if (status != FAT_SUCCESS) {
        return status;
    }

    /* Initialize file handle */
    file->cluster = ((uint32_t)entry.first_cluster_hi << 16) | entry.first_cluster_lo;
    file->position = 0;
    file->size = entry.file_size;
    file->mode = mode;
    file->current_sector = fat_driver_cluster_to_sector(&fat_ctx, file->cluster);
    file->sector_offset = 0;
    file->sector_buffer = NULL;
    file->is_dirty = false;
    strncpy(file->name, path, sizeof(file->name) - 1);
    file->name[sizeof(file->name) - 1] = '\0';

    return FAT_SUCCESS;
}

int32_t fat_close(fat_file_t *file)
{
    if (!file) {
        return FAT_ERROR_INVALID;
    }

    /* Free sector buffer */
    if (file->sector_buffer) {
        free(file->sector_buffer);
    }

    return FAT_SUCCESS;
}

int32_t fat_read(fat_file_t *file, void *buffer, uint32_t size, uint32_t *bytes_read)
{
    if (!file || !buffer || !bytes_read) {
        return FAT_ERROR_INVALID;
    }

    if (file->position >= file->size) {
        *bytes_read = 0;
        return FAT_SUCCESS;
    }

    uint32_t bytes_to_read = size;
    if (file->position + bytes_to_read > file->size) {
        bytes_to_read = file->size - file->position;
    }

    uint32_t bytes_remaining = bytes_to_read;
    uint8_t *ptr = buffer;

    while (bytes_remaining > 0) {
        /* Allocate sector buffer if needed */
        if (!file->sector_buffer) {
            file->sector_buffer = malloc(FAT_SECTOR_SIZE);
            if (!file->sector_buffer) {
                return FAT_ERROR_IO;
            }
        }

        /* Read sector if needed */
        if (file->sector_offset == 0) {
            int32_t status = fat_driver_read_sector(&fat_ctx, file->current_sector, file->sector_buffer);
            if (status != FAT_SUCCESS) {
                return status;
            }
        }

        /* Copy data from sector buffer */
        uint32_t bytes_in_sector = FAT_SECTOR_SIZE - file->sector_offset;
        uint32_t bytes_to_copy = bytes_remaining < bytes_in_sector ? bytes_remaining : bytes_in_sector;

        memcpy(ptr, &file->sector_buffer[file->sector_offset], bytes_to_copy);
        ptr += bytes_to_copy;
        bytes_remaining -= bytes_to_copy;
        file->position += bytes_to_copy;
        file->sector_offset += bytes_to_copy;

        /* Move to next sector if needed */
        if (file->sector_offset >= FAT_SECTOR_SIZE) {
            file->sector_offset = 0;
            file->current_sector++;

            /* Move to next cluster if needed */
            if (file->current_sector >= fat_ctx.sectors_per_cluster) {
                file->current_sector = 0;
                uint32_t next_cluster;
                int32_t status = fat_driver_get_next_cluster(&fat_ctx, file->cluster, &next_cluster);
                if (status != FAT_SUCCESS) {
                    return status;
                }

                if (fat_driver_is_eof_cluster(&fat_ctx, next_cluster)) {
                    break;
                }

                file->cluster = next_cluster;
                file->current_sector = fat_driver_cluster_to_sector(&fat_ctx, file->cluster);
            }
        }
    }

    *bytes_read = bytes_to_read - bytes_remaining;
    return FAT_SUCCESS;
}

int32_t fat_seek(fat_file_t *file, int32_t offset, uint8_t whence)
{
    if (!file) {
        return FAT_ERROR_INVALID;
    }

    uint32_t new_position;

    switch (whence) {
        case SEEK_SET:
            new_position = offset;
            break;

        case SEEK_CUR:
            new_position = file->position + offset;
            break;

        case SEEK_END:
            new_position = file->size + offset;
            break;

        default:
            return FAT_ERROR_INVALID;
    }

    if (new_position > file->size) {
        return FAT_ERROR_INVALID;
    }

    /* Calculate new cluster and sector */
    uint32_t cluster_size = FAT_SECTOR_SIZE * fat_ctx.sectors_per_cluster;
    uint32_t new_cluster = file->cluster;
    uint32_t clusters_to_skip = new_position / cluster_size;

    while (clusters_to_skip > 0) {
        uint32_t next_cluster;
        int32_t status = fat_driver_get_next_cluster(&fat_ctx, new_cluster, &next_cluster);
        if (status != FAT_SUCCESS) {
            return status;
        }

        if (fat_driver_is_eof_cluster(&fat_ctx, next_cluster)) {
            return FAT_ERROR_INVALID;
        }

        new_cluster = next_cluster;
        clusters_to_skip--;
    }

    uint32_t new_sector = fat_driver_cluster_to_sector(&fat_ctx, new_cluster);
    uint32_t new_offset = new_position % FAT_SECTOR_SIZE;

    file->cluster = new_cluster;
    file->current_sector = new_sector;
    file->sector_offset = new_offset;
    file->position = new_position;

    return FAT_SUCCESS;
}

int32_t fat_opendir(const char *path, fat_dir_t *dir)
{
    if (!path || !dir) {
        return FAT_ERROR_INVALID;
    }

    fat_dir_entry_t entry;
    int32_t status = fat_driver_find_dir_entry(&fat_ctx, path, &entry);
    if (status != FAT_SUCCESS) {
        return status;
    }

    if (!(entry.attr & FAT_ATTR_DIRECTORY)) {
        return FAT_ERROR_ACCESS_DENIED;
    }

    dir->cluster = (uint32_t)entry.first_cluster_hi << 16 | entry.first_cluster_lo;
    dir->current_sector = fat_driver_cluster_to_sector(&fat_ctx, dir->cluster);
    dir->sector_offset = 0;
    dir->sector_buffer = NULL;

    return FAT_SUCCESS;
}

int32_t fat_closedir(fat_dir_t *dir)
{
    if (!dir) {
        return FAT_ERROR_INVALID;
    }

    if (dir->sector_buffer) {
        free(dir->sector_buffer);
        dir->sector_buffer = NULL;
    }

    return FAT_SUCCESS;
}

int32_t fat_readdir(fat_dir_t *dir, fat_dir_entry_t *entry)
{
    if (!dir || !entry) {
        return FAT_ERROR_INVALID;
    }

    while (1) {
        /* Allocate sector buffer if needed */
        if (!dir->sector_buffer) {
            dir->sector_buffer = malloc(FAT_SECTOR_SIZE);
            if (!dir->sector_buffer) {
                return FAT_ERROR_IO;
            }
        }

        /* Read sector if needed */
        if (dir->sector_offset == 0) {
            int32_t status = fat_driver_read_sector(&fat_ctx, dir->current_sector, dir->sector_buffer);
            if (status != FAT_SUCCESS) {
                return status;
            }
        }

        /* Get directory entry */
        fat_dir_entry_t *dir_entry = (fat_dir_entry_t *)&dir->sector_buffer[dir->sector_offset];

        /* Check if end of directory */
        if (dir_entry->name[0] == 0) {
            return FAT_ERROR_NOT_FOUND;
        }

        /* Skip deleted entries */
        if (dir_entry->name[0] != FAT_DIR_DELETED) {
            /* Copy entry */
            memcpy(entry, dir_entry, sizeof(fat_dir_entry_t));
            dir->sector_offset += sizeof(fat_dir_entry_t);
            if (dir->sector_offset >= FAT_SECTOR_SIZE) {
                dir->sector_offset = 0;
                dir->current_sector++;

                /* Move to next cluster if needed */
                if (dir->current_sector >= fat_ctx.sectors_per_cluster) {
                    dir->current_sector = 0;

                    uint32_t next_cluster;
                    int32_t status = fat_driver_get_next_cluster(&fat_ctx, dir->cluster, &next_cluster);
                    if (status != FAT_SUCCESS) {
                        return status;
                    }

                    if (fat_driver_is_eof_cluster(&fat_ctx, next_cluster)) {
                        return FAT_ERROR_NOT_FOUND;
                    }

                    dir->cluster = next_cluster;
                    dir->current_sector = fat_driver_cluster_to_sector(&fat_ctx, dir->cluster);
                }
            }

            return FAT_SUCCESS;
        }

        /* Move to next entry */
        dir->sector_offset += sizeof(fat_dir_entry_t);
        if (dir->sector_offset >= FAT_SECTOR_SIZE) {
            dir->sector_offset = 0;
            dir->current_sector++;

            /* Move to next cluster if needed */
            if (dir->current_sector >= fat_ctx.sectors_per_cluster) {
                dir->current_sector = 0;

                uint32_t next_cluster;
                int32_t status = fat_driver_get_next_cluster(&fat_ctx, dir->cluster, &next_cluster);
                if (status != FAT_SUCCESS) {
                    return status;
                }

                if (fat_driver_is_eof_cluster(&fat_ctx, next_cluster)) {
                    return FAT_ERROR_NOT_FOUND;
                }

                dir->cluster = next_cluster;
                dir->current_sector = fat_driver_cluster_to_sector(&fat_ctx, dir->cluster);
            }
        }
    }
}

int32_t fat_rewinddir(fat_dir_t *dir)
{
    if (!dir) {
        return FAT_ERROR_INVALID;
    }

    dir->sector_offset = 0;
    dir->current_sector = fat_driver_cluster_to_sector(&fat_ctx, dir->cluster);

    return FAT_SUCCESS;
}

int32_t fat_stat(const char *path, fat_dir_entry_t *entry)
{
    if (!path || !entry) {
        return FAT_ERROR_INVALID;
    }

    return fat_driver_find_dir_entry(&fat_ctx, path, entry);
}

int32_t fat_fstat(fat_file_t *file, fat_dir_entry_t *entry)
{
    if (!file || !entry) {
        return FAT_ERROR_INVALID;
    }

    return fat_stat(file->name, entry);
}

int32_t fat_access(const char *path, uint8_t mode)
{
    if (!path) {
        return FAT_ERROR_INVALID;
    }

    fat_dir_entry_t entry;
    int32_t status = fat_driver_find_dir_entry(&fat_ctx, path, &entry);
    if (status != FAT_SUCCESS) {
        return status;
    }

    if ((mode & FAT_MODE_WRITE) && (entry.attr & FAT_ATTR_READ_ONLY)) {
        return FAT_ERROR_ACCESS_DENIED;
    }

    return FAT_SUCCESS;
}

/*********************************************************************
 * Private Function Implementations
 *********************************************************************/

static int32_t fat_driver_init_storage(void)
{
    return FAT_SUCCESS;
}

static int32_t fat_driver_validate_boot_sector(void)
{
    return FAT_SUCCESS;
}

static int32_t fat_driver_init_fat_tables(void)
{
    return FAT_SUCCESS;
}

static uint32_t fat_driver_get_fat_size(const fat_boot_sector_t *boot_sector)
{
    return boot_sector->fat_size_16 ? boot_sector->fat_size_16 : boot_sector->fat_size_32;
}

static int32_t fat_driver_calculate_layout(const fat_boot_sector_t *boot_sector)
{
    return FAT_SUCCESS;
}

static int32_t fat_driver_is_directory(const fat_dir_entry_t *entry)
{
    return (entry->attr & FAT_ATTR_DIRECTORY) ? FAT_SUCCESS : FAT_ERROR;
}

static int32_t fat_driver_list_directory(const char *path)
{
    return FAT_SUCCESS;
}

static int32_t fat_driver_change_directory(const char *path)
{
    return FAT_SUCCESS;
}

static int32_t fat_driver_resolve_path(const char *path, char *resolved_path)
{
    return FAT_SUCCESS;
}

static int32_t fat_driver_normalize_path(const char *path, char *normalized_path)
{
    return FAT_SUCCESS;
}

static int32_t fat_driver_get_parent_path(const char *path, char *parent_path)
{
    return FAT_SUCCESS;
}

static int32_t fat_driver_init_path_context(void)
{
    return FAT_SUCCESS;
}

static int32_t fat_driver_update_path_context(const char *new_path, uint32_t new_cluster)
{
    return FAT_SUCCESS;
}

/*********************************************************************
 * UUID: 2b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/