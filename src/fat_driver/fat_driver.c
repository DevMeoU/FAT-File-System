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
int32_t fat_driver_init_storage(void);
int32_t fat_driver_read_boot_sector(void);
int32_t fat_driver_validate_boot_sector(void);
int32_t fat_driver_init_fat_tables(void);
uint32_t fat_driver_get_fat_size(const fat_boot_sector_t *boot_sector);
int32_t fat_driver_calculate_layout(const fat_boot_sector_t *boot_sector);

/* Private functions from fat_driver.c */
static int32_t fat_driver_read_sector(uint32_t sector, uint8_t *buffer);
static uint32_t fat_driver_cluster_to_sector(uint32_t cluster);
static uint16_t fat_driver_get_time(void);
static uint16_t fat_driver_get_date(void);
static int32_t fat_driver_is_directory(const fat_entry_t *entry);
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
fat_context_t fat_ctx;
fat_boot_sector_t boot_sector;
fat_type_t fat_type;
static fat_path_context_t path_ctx;

/*********************************************************************
 * Public Function Implementations
 *********************************************************************/

int32_t fat_init(const char* img_path)
{
    int32_t status = FAT_ERROR_INVALID;

    /* Copy image path */
    strncpy(fat_ctx.config.img_path, img_path, sizeof(fat_ctx.config.img_path) - 1);
    fat_ctx.config.img_path[sizeof(fat_ctx.config.img_path) - 1] = '\0';

    /* Read boot sector */
    status = fat_driver_read_boot_sector(&fat_ctx);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    /* Read FAT table */
    status = fat_driver_read_fat_table(&fat_ctx);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    /* Read root directory */
    status = fat_driver_read_root_dir(&fat_ctx);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    /* Initialize path context */
    status = fat_driver_init_path_context();
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    return FAT_ERROR_SUCCESS;
}

int32_t fat_open(const char *path, uint8_t mode, fat_file_t *file)
{
    if (!path || !file) {
        return FAT_ERROR_INVALID;
    }

    /* Initialize file structure */
    memset(file, 0, sizeof(fat_file_t));
    file->mode = mode;

    /* Find or create file */
    fat_entry_t entry;
    int32_t status = fat_driver_find_dir_entry(&fat_ctx, path, &entry);
    if (status != FAT_ERROR_SUCCESS) {
        if (mode & FAT_MODE_CREATE) {
            status = fat_driver_create_file(&fat_ctx, path, FAT_ATTR_ARCHIVE);
        } else {
            return status;
        }
    }

    /* Check file attributes */
    if (entry.attributes & FAT_ATTR_READ_ONLY && (mode & FAT_MODE_WRITE)) {
        return FAT_ERROR_ACCESS_DENIED;
    }

    /* Copy file information */
    file->info.attributes = entry.attributes;
    file->info.size = entry.file_size;
    file->info.cluster = (entry.first_cluster_high << 16) | entry.first_cluster_low;
    strncpy((char *)file->info.name, entry.name, sizeof(file->info.name) - 1);
    file->info.name[sizeof(file->info.name) - 1] = '\0';

    /* Initialize file position */
    file->position = 0;
    file->cluster = file->info.cluster;
    file->sector = fat_driver_cluster_to_sector(file->cluster);
    file->offset = 0;

    return FAT_ERROR_SUCCESS;
}

void fat_close(fat_file_t *file)
{
    if (!file) {
        return;
    }

    /* Update directory entry if file was modified */
    if (file->modified) {
        fat_entry_t entry;
        int32_t status = fat_driver_find_dir_entry(&fat_ctx, (char *)file->info.name, &entry);
        if (status == FAT_ERROR_SUCCESS) {
            entry.file_size = file->info.size;
            entry.first_cluster_high = (uint16_t)(file->info.cluster >> 16);
            entry.first_cluster_low = (uint16_t)file->info.cluster;
            entry.write_date = file->info.date;
            entry.write_time = file->info.time;
            fat_driver_update_dir_entry(&fat_ctx, (char *)file->info.name, &entry);
        }
    }

    /* Write any remaining data */
    if (file->offset > 0) {
        fat_driver_cache_write(&fat_ctx, file->sector, file->sector_buffer);
    }
}

int32_t fat_read(fat_file_t *file, void *buffer, uint32_t size, uint32_t *bytes_read)
{
    if (!file || !buffer || !bytes_read) {
        return FAT_ERROR_INVALID;
    }

    /* Check file mode */
    if (!(file->mode & FAT_MODE_READ)) {
        return FAT_ERROR_ACCESS_DENIED;
    }

    /* Check end of file */
    if (file->position >= file->info.size) {
        *bytes_read = 0;
        return FAT_ERROR_SUCCESS;
    }

    /* Calculate bytes to read */
    uint32_t bytes_to_read = size;
    if (file->position + bytes_to_read > file->info.size) {
        bytes_to_read = file->info.size - file->position;
    }

    /* Read data */
    uint8_t *ptr = (uint8_t *)buffer;
    uint32_t bytes_remaining = bytes_to_read;

    while (bytes_remaining > 0) {
        /* Read sector if needed */
        if (file->offset == 0) {
            int32_t status = fat_driver_read_sector(file->sector, file->sector_buffer);
            if (status != FAT_ERROR_SUCCESS) {
                return status;
            }
        }

        /* Calculate bytes to copy */
        uint32_t bytes_in_sector = FAT_SECTOR_SIZE - file->offset;
        uint32_t bytes_to_copy = (bytes_remaining < bytes_in_sector) ? bytes_remaining : bytes_in_sector;

        /* Copy data */
        memcpy(ptr, &file->sector_buffer[file->offset], bytes_to_copy);

        /* Update pointers */
        ptr += bytes_to_copy;
        file->position += bytes_to_copy;
        file->offset += bytes_to_copy;
        bytes_remaining -= bytes_to_copy;

        /* Move to next sector if needed */
        if (file->offset >= FAT_SECTOR_SIZE) {
            file->offset = 0;
            file->sector++;

            /* Move to next cluster if needed */
            if (file->sector >= fat_ctx.config.sectors_per_cluster) {
                file->sector = 0;
                uint32_t next_cluster;
                int32_t status = fat_driver_get_next_cluster(file->cluster, &next_cluster);
                if (status != FAT_ERROR_SUCCESS) {
                    return status;
                }

                /* Check end of chain */
                if (next_cluster >= FAT_EOC(fat_ctx.config.fat_type)) {
                    *bytes_read = bytes_to_read - bytes_remaining;
                    return FAT_ERROR_SUCCESS;
                }

                file->cluster = next_cluster;
                file->sector = fat_driver_cluster_to_sector(file->cluster);
            }
        }
    }

    *bytes_read = bytes_to_read;
    return FAT_ERROR_SUCCESS;
}

int32_t fat_write(fat_file_t *file, const void *buffer, uint32_t size, uint32_t *bytes_written)
{
    if (!file || !buffer || !bytes_written) {
        return FAT_ERROR_INVALID;
    }

    /* Check file mode */
    if (!(file->mode & FAT_MODE_WRITE)) {
        return FAT_ERROR_ACCESS_DENIED;
    }

    /* Check mount mode */
    if (fat_ctx.mount_mode & FAT_MOUNT_READ_ONLY) {
        return FAT_ERROR_ACCESS_DENIED;
    }

    /* Write data */
    const uint8_t *ptr = (const uint8_t *)buffer;
    uint32_t bytes_remaining = size;

    while (bytes_remaining > 0) {
        /* Read sector if needed */
        if (file->offset == 0) {
            int32_t status = fat_driver_read_sector(file->sector, file->sector_buffer);
            if (status != FAT_ERROR_SUCCESS) {
                return status;
            }
        }

        /* Calculate bytes to copy */
        uint32_t bytes_in_sector = FAT_SECTOR_SIZE - file->offset;
        uint32_t bytes_to_copy = (bytes_remaining < bytes_in_sector) ? bytes_remaining : bytes_in_sector;

        /* Copy data */
        memcpy(&file->sector_buffer[file->offset], ptr, bytes_to_copy);

        /* Update pointers */
        ptr += bytes_to_copy;
        file->position += bytes_to_copy;
        file->offset += bytes_to_copy;
        bytes_remaining -= bytes_to_copy;

        /* Write sector if full */
        if (file->offset >= FAT_SECTOR_SIZE) {
            int32_t status = fat_driver_write_sector(file->sector, file->sector_buffer);
            if (status != FAT_ERROR_SUCCESS) {
                return status;
            }

            file->offset = 0;
            file->sector++;

            /* Move to next cluster if needed */
            if (file->sector >= fat_ctx.config.sectors_per_cluster) {
                file->sector = 0;
                uint32_t next_cluster;
                int32_t status = fat_driver_get_next_cluster(file->cluster, &next_cluster);
                if (status != FAT_ERROR_SUCCESS) {
                    return status;
                }

                /* Check end of chain */
                if (next_cluster >= FAT_EOC(fat_ctx.config.fat_type)) {
                    /* Allocate new cluster */
                    status = fat_driver_find_free_cluster(&next_cluster);
                    if (status != FAT_ERROR_SUCCESS) {
                        return status;
                    }

                    /* Update FAT */
                    status = fat_driver_set_next_cluster(file->cluster, next_cluster);
                    if (status != FAT_ERROR_SUCCESS) {
                        return status;
                    }

                    /* Clear new cluster */
                    memset(file->sector_buffer, 0, FAT_SECTOR_SIZE);
                    for (uint32_t i = 0; i < fat_ctx.config.sectors_per_cluster; i++) {
                        status = fat_driver_write_sector(fat_driver_cluster_to_sector(next_cluster) + i,
                                                        file->sector_buffer);
                        if (status != FAT_ERROR_SUCCESS) {
                            return status;
                        }
                    }
                }

                file->cluster = next_cluster;
                file->sector = fat_driver_cluster_to_sector(file->cluster);
            }
        }
    }

    /* Write remaining data */
    if (file->offset > 0) {
        int32_t status = fat_driver_write_sector(file->sector, file->sector_buffer);
        if (status != FAT_ERROR_SUCCESS) {
            return status;
        }
    }

    /* Update file size */
    if (file->position > file->info.size) {
        file->info.size = file->position;
        file->modified = true;
    }

    *bytes_written = size;
    return FAT_ERROR_SUCCESS;
}

int32_t fat_seek(fat_file_t *file, int32_t offset, uint8_t whence)
{
    if (!file) {
        return FAT_ERROR_INVALID;
    }

    /* Calculate new position */
    uint32_t new_position;
    switch (whence) {
        case FAT_SEEK_SET:
            new_position = offset;
            break;
        case FAT_SEEK_CUR:
            new_position = file->position + offset;
            break;
        case FAT_SEEK_END:
            new_position = file->info.size + offset;
            break;
        default:
            return FAT_ERROR_INVALID;
    }

    /* Check bounds */
    if (new_position > file->info.size) {
        return FAT_ERROR_INVALID;
    }

    /* Calculate new cluster and sector */
    uint32_t new_cluster = file->info.cluster;
    uint32_t new_sector = fat_driver_cluster_to_sector(new_cluster);
    uint32_t new_offset = new_position % FAT_SECTOR_SIZE;

    /* Follow cluster chain */
    uint32_t cluster_offset = new_position / (FAT_SECTOR_SIZE * fat_ctx.config.sectors_per_cluster);
    for (uint32_t i = 0; i < cluster_offset; i++) {
        uint32_t next_cluster;
        int32_t status = fat_driver_get_next_cluster(new_cluster, &next_cluster);
        if (status != FAT_ERROR_SUCCESS) {
            return status;
        }

        if (next_cluster >= FAT_EOC(fat_ctx.config.fat_type)) {
            return FAT_ERROR_INVALID;
        }

        new_cluster = next_cluster;
        new_sector = fat_driver_cluster_to_sector(new_cluster);
    }

    /* Update file position */
    file->position = new_position;
    file->cluster = new_cluster;
    file->sector = new_sector;
    file->offset = new_offset;

    return FAT_ERROR_SUCCESS;
}

int32_t fat_unlink(const char *path)
{
    if (!path) {
        return FAT_ERROR_INVALID;
    }

    /* Check mount mode */
    if (fat_ctx.mount_mode & FAT_MOUNT_READ_ONLY) {
        return FAT_ERROR_ACCESS_DENIED;
    }

    /* Find file */
    fat_entry_t entry;
    int32_t status = fat_driver_find_dir_entry(&fat_ctx, path, &entry);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    /* Check attributes */
    if (entry.attributes & FAT_ATTR_READ_ONLY) {
        return FAT_ERROR_ACCESS_DENIED;
    }

    /* Free clusters */
    uint32_t cluster = (entry.first_cluster_high << 16) | entry.first_cluster_low;
    while (cluster < FAT_EOC(fat_ctx.config.fat_type)) {
        uint32_t next_cluster;
        status = fat_driver_get_next_cluster(cluster, &next_cluster);
        if (status != FAT_ERROR_SUCCESS) {
            return status;
        }

        status = fat_driver_free_cluster(cluster);
        if (status != FAT_ERROR_SUCCESS) {
            return status;
        }

        cluster = next_cluster;
    }

    /* Mark directory entry as deleted */
    entry.name[0] = FAT_DIR_DELETED;
    status = fat_driver_update_dir_entry(&fat_ctx, path, &entry);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    return FAT_ERROR_SUCCESS;
}

int32_t fat_mkdir(const char *path)
{
    if (!path) {
        return FAT_ERROR_INVALID;
    }

    /* Check mount mode */
    if (fat_ctx.mount_mode & FAT_MOUNT_READ_ONLY) {
        return FAT_ERROR_ACCESS_DENIED;
    }

    /* Find parent directory */
    fat_entry_t entry;
    int32_t status = fat_driver_find_dir_entry(&fat_ctx, path, &entry);
    if (status == FAT_ERROR_SUCCESS) {
        return FAT_ERROR_ALREADY_EXISTS;
    }

    /* Create directory entry */
    memset(&entry, 0, sizeof(entry));
    entry.attributes = FAT_ATTR_DIRECTORY;
    entry.first_cluster_high = 0;
    entry.first_cluster_low = 0;
    entry.file_size = 0;
    entry.write_date = fat_driver_get_date();
    entry.write_time = fat_driver_get_time();

    /* Convert name */
    char short_name[11];
    status = fat_driver_convert_to_short_name(path, short_name);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }
    memcpy(entry.name, short_name, 11);

    /* Allocate cluster */
    uint32_t cluster;
    status = fat_driver_alloc_cluster();
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    /* Update directory entry */
    entry.first_cluster_high = (uint16_t)(cluster >> 16);
    entry.first_cluster_low = (uint16_t)cluster;
    status = fat_driver_update_dir_entry(&fat_ctx, path, &entry);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    return FAT_ERROR_SUCCESS;
}

int32_t fat_rmdir(const char *path)
{
    if (!path) {
        return FAT_ERROR_INVALID;
    }

    /* Check mount mode */
    if (fat_ctx.mount_mode & FAT_MOUNT_READ_ONLY) {
        return FAT_ERROR_ACCESS_DENIED;
    }

    /* Find directory */
    fat_entry_t entry;
    int32_t status = fat_driver_find_dir_entry(&fat_ctx, path, &entry);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    /* Check attributes */
    if (!(entry.attributes & FAT_ATTR_DIRECTORY)) {
        return FAT_ERROR_INVALID;
    }

    /* Check if directory is empty */
    uint32_t cluster = (entry.first_cluster_high << 16) | entry.first_cluster_low;
    uint8_t sector_buffer[FAT_SECTOR_SIZE];
    fat_entry_t *dir_entry = (fat_entry_t *)sector_buffer;

    while (cluster < FAT_EOC(fat_ctx.config.fat_type)) {
        for (uint32_t i = 0; i < fat_ctx.config.sectors_per_cluster; i++) {
            status = fat_driver_read_sector(fat_driver_cluster_to_sector(cluster) + i, sector_buffer);
            if (status != FAT_ERROR_SUCCESS) {
                return status;
            }

            for (uint32_t j = 0; j < FAT_SECTOR_SIZE / sizeof(fat_entry_t); j++) {
                if (dir_entry[j].name[0] != 0 && dir_entry[j].name[0] != FAT_DIR_DELETED) {
                    return FAT_ERROR_NOT_EMPTY;
                }
            }
        }

        status = fat_driver_get_next_cluster(cluster, &cluster);
        if (status != FAT_ERROR_SUCCESS) {
            return status;
        }
    }

    /* Free clusters */
    cluster = (entry.first_cluster_high << 16) | entry.first_cluster_low;
    while (cluster < FAT_EOC(fat_ctx.config.fat_type)) {
        uint32_t next_cluster;
        status = fat_driver_get_next_cluster(cluster, &next_cluster);
        if (status != FAT_ERROR_SUCCESS) {
            return status;
        }

        status = fat_driver_free_cluster(cluster);
        if (status != FAT_ERROR_SUCCESS) {
            return status;
        }

        cluster = next_cluster;
    }

    /* Mark directory entry as deleted */
    entry.name[0] = FAT_DIR_DELETED;
    status = fat_driver_update_dir_entry(&fat_ctx, path, &entry);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    return FAT_ERROR_SUCCESS;
}

int32_t fat_opendir(const char *path, fat_dir_t *dir)
{
    if (!path || !dir) {
        return FAT_ERROR_INVALID;
    }

    /* Find directory */
    fat_entry_t entry;
    int32_t status = fat_driver_find_dir_entry(&fat_ctx, path, &entry);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    /* Check attributes */
    if (!(entry.attributes & FAT_ATTR_DIRECTORY)) {
        return FAT_ERROR_INVALID;
    }

    /* Initialize directory structure */
    memset(dir, 0, sizeof(fat_dir_t));
    dir->cluster = (entry.first_cluster_high << 16) | entry.first_cluster_low;
    dir->sector = fat_driver_cluster_to_sector(dir->cluster);
    dir->offset = 0;

    return FAT_ERROR_SUCCESS;
}

void fat_closedir(fat_dir_t *dir)
{
    if (!dir) {
        return;
    }

    memset(dir, 0, sizeof(fat_dir_t));
}

int32_t fat_readdir(fat_dir_t *dir, fat_entry_t *entry)
{
    if (!dir || !entry) {
        return FAT_ERROR_INVALID;
    }

    /* Read sector if needed */
    if (dir->offset == 0) {
        int32_t status = fat_driver_read_sector(dir->sector, dir->sector_buffer);
        if (status != FAT_ERROR_SUCCESS) {
            return status;
        }
    }

    /* Get directory entry */
    fat_entry_t *dir_entry = (fat_entry_t *)&dir->sector_buffer[dir->offset];
    if (dir_entry->name[0] == 0) {
        return FAT_ERROR_NOT_FOUND;
    }

    if (dir_entry->name[0] == FAT_DIR_DELETED) {
        dir->offset += sizeof(fat_entry_t);
        if (dir->offset >= FAT_SECTOR_SIZE) {
            dir->offset = 0;
            dir->sector++;
        }
        return fat_readdir(dir, entry);
    }

    /* Copy entry */
    memcpy(entry, dir_entry, sizeof(fat_entry_t));

    /* Update directory position */
    dir->offset += sizeof(fat_entry_t);
    if (dir->offset >= FAT_SECTOR_SIZE) {
        dir->offset = 0;
        dir->sector++;

        /* Move to next cluster if needed */
        if (dir->sector >= fat_ctx.config.sectors_per_cluster) {
            dir->sector = 0;
            uint32_t next_cluster;
            int32_t status = fat_driver_get_next_cluster(dir->cluster, &next_cluster);
            if (status != FAT_ERROR_SUCCESS) {
                return status;
            }

            /* Check end of chain */
            if (next_cluster >= FAT_EOC(fat_ctx.config.fat_type)) {
                return FAT_ERROR_NOT_FOUND;
            }

            dir->cluster = next_cluster;
            dir->sector = fat_driver_cluster_to_sector(dir->cluster);
        }
    }

    return FAT_ERROR_SUCCESS;
}

int32_t fat_rewinddir(fat_dir_t *dir)
{
    if (!dir) {
        return FAT_ERROR_INVALID;
    }

    dir->offset = 0;
    dir->sector = fat_driver_cluster_to_sector(dir->cluster);

    return FAT_ERROR_SUCCESS;
}

int32_t fat_stat(const char *path, fat_entry_t *entry)
{
    if (!path || !entry) {
        return FAT_ERROR_INVALID;
    }

    return fat_driver_find_dir_entry(&fat_ctx, path, entry);
}

int32_t fat_fstat(fat_file_t *file, fat_entry_t *entry)
{
    if (!file || !entry) {
        return FAT_ERROR_INVALID;
    }

    /* Copy file information */
    entry->attributes = file->info.attributes;
    entry->file_size = file->info.size;
    entry->first_cluster_high = (uint16_t)(file->info.cluster >> 16);
    entry->first_cluster_low = (uint16_t)file->info.cluster;
    entry->write_date = file->info.date;
    entry->write_time = file->info.time;
    strncpy(entry->name, (char *)file->info.name, sizeof(entry->name) - 1);
    entry->name[sizeof(entry->name) - 1] = '\0';

    return FAT_ERROR_SUCCESS;
}

int32_t fat_access(const char *path, uint8_t mode)
{
    if (!path) {
        return FAT_ERROR_INVALID;
    }

    /* Find file */
    fat_entry_t entry;
    int32_t status = fat_driver_find_dir_entry(&fat_ctx, path, &entry);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    /* Check attributes */
    if (entry.attributes & FAT_ATTR_READ_ONLY && (mode & FAT_MODE_WRITE)) {
        return FAT_ERROR_ACCESS_DENIED;
    }

    return FAT_ERROR_SUCCESS;
}

int32_t fat_mount(const char *path, uint32_t mode)
{
    if (!path) {
        return FAT_ERROR_INVALID;
    }

    /* Copy image path */
    strncpy(fat_ctx.config.img_path, path, sizeof(fat_ctx.config.img_path) - 1);
    fat_ctx.config.img_path[sizeof(fat_ctx.config.img_path) - 1] = '\0';

    /* Set mount mode */
    fat_ctx.mount_mode = mode;

    /* Read boot sector */
    int32_t status = fat_driver_read_boot_sector(&fat_ctx);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    /* Read FAT table */
    status = fat_driver_read_fat_table(&fat_ctx);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    /* Read root directory */
    status = fat_driver_read_root_dir(&fat_ctx);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    return FAT_ERROR_SUCCESS;
}

int32_t fat_umount(void)
{
    /* Flush cache */
    int32_t status = fat_driver_cache_flush(&fat_ctx);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    /* Free memory */
    if (fat_ctx.fat_table) {
        free(fat_ctx.fat_table);
        fat_ctx.fat_table = NULL;
    }

    if (fat_ctx.root_dir) {
        free(fat_ctx.root_dir);
        fat_ctx.root_dir = NULL;
    }

    return FAT_ERROR_SUCCESS;
}

int32_t fat_sync(void)
{
    return fat_driver_cache_flush(&fat_ctx);
}

/*********************************************************************
 * UUID: 2b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/