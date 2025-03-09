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

/*********************************************************************
 * Private Function Prototypes
 *********************************************************************/
/* Private functions from fat_driver_private.c */
int32_t fat_init_storage(void);
int32_t fat_read_boot_sector(void);
int32_t fat_validate_boot_sector(void);
int32_t fat_init_fat_tables(void);
uint32_t get_fat_size(const fat_boot_sector_t *boot_sector);
void calculate_layout(const fat_boot_sector_t *boot_sector);

/* Private functions from fat_driver.c */
static int32_t read_sector(uint32_t sector, uint8_t *buffer);
static uint32_t fat_cluster_to_sector(uint32_t cluster);
static uint16_t fat_get_time(void);
static uint16_t fat_get_date(void);
static int32_t is_directory(const fat_dir_entry_t *entry);
static int32_t list_directory(const char *path);
static int32_t change_directory(const char *path);
static int32_t resolve_path(const char *path, char *resolved_path);
static int32_t normalize_path(const char *path, char *normalized_path);
static int32_t get_parent_path(const char *path, char *parent_path);
static int32_t init_path_context(void);
static int32_t update_path_context(const char *new_path, uint32_t new_cluster);

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

int32_t fat_init(const fat_config_t *config)
{
    if (config == NULL) {
        return STATUS_INVALID;
    }

    /* Initialize context */
    memset(&fat_ctx, 0, sizeof(fat_context_t));
    memset(&path_ctx, 0, sizeof(path_ctx));

    /* Copy configuration */
    memcpy(&fat_ctx.config, config, sizeof(fat_config_t));

    /* Initialize storage */
    int32_t status = fat_init_storage();
    if (status != STATUS_SUCCESS) {
        return status;
    }

    /* Read boot sector */
    status = fat_read_boot_sector();
    if (status != STATUS_SUCCESS) {
        return status;
    }

    /* Validate boot sector */
    status = fat_validate_boot_sector();
    if (status != STATUS_SUCCESS) {
        return status;
    }

    /* Calculate layout */
    calculate_layout(&boot_sector);

    /* Initialize FAT tables */
    status = fat_init_fat_tables();
    if (status != STATUS_SUCCESS) {
        return status;
    }

    /* Initialize path context */
    status = init_path_context();
    if (status != STATUS_SUCCESS) {
        return status;
    }

    return STATUS_SUCCESS;
}

int32_t fat_open(const char *path, uint8_t mode, fat_file_t *file)
{
    if (path == NULL || file == NULL) {
        return STATUS_INVALID;
    }

    /* Initialize file handle */
    memset(file, 0, sizeof(fat_file_t));
    file->mode = mode;

    /* Tìm và tính entry cho file */
    fat_dir_entry_t entry;
    memset(&entry, 0, sizeof(fat_dir_entry_t));
    
    /* Tìm file trong thư mục */
    int32_t status = fat_find_file(path, &entry);
    
    /* Xử lý trường hợp file không tồn tại */
    if (status == STATUS_NOT_FOUND) {
        if (mode & FAT_MODE_CREATE) {
            /* Tạo file mới nếu có quyền tạo */
            status = fat_create_file(path, &entry);
            if (status != STATUS_SUCCESS) {
                return status; /* Lỗi khi tạo file */
            }
        } else {
            return STATUS_NOT_FOUND; /* Không tìm thấy file */
        }
    } else if (status != STATUS_SUCCESS) {
        return status; /* Lỗi khác */
    }

    /* Check access mode */
    if ((entry.attributes & FAT_ATTR_READ_ONLY) && (mode & FAT_MODE_WRITE)) {
        return STATUS_READ_ONLY;
    }

    /* Set file info */
    file->info.attributes = entry.attributes;
    file->info.size = entry.file_size;
    file->info.cluster = (entry.first_cluster_hi << 16) | entry.first_cluster_lo;
    file->info.date = fat_get_date();
    file->info.time = fat_get_time();
    fat_get_name(&entry, (char *)file->info.name);

    /* Initialize position */
    file->position = 0;
    file->cluster = file->info.cluster;
    file->sector = fat_cluster_to_sector(file->cluster);
    file->offset = 0;

    return STATUS_SUCCESS;
}

int32_t fat_close(fat_file_t *file)
{
    if (file == NULL) {
        return STATUS_INVALID;
    }

    /* Update file info if modified */
    if (file->modified) {
        fat_dir_entry_t entry;
        int32_t status = fat_find_file((char *)file->info.name, &entry);
        if (status == STATUS_SUCCESS) {
            entry.file_size = file->info.size;
            entry.first_cluster_hi = (uint16_t)(file->info.cluster >> 16);
            entry.first_cluster_lo = (uint16_t)file->info.cluster;
            entry.last_write_date = file->info.date;
            entry.last_write_time = file->info.time;

            /* Write entry back to directory */
            status = fat_write_dir_entry(&entry);
            if (status != STATUS_SUCCESS) {
                return status;
            }
        }
    }

    return STATUS_SUCCESS;
}

int32_t fat_read(fat_file_t *file, void *buffer, uint32_t size, uint32_t *bytes_read)
{
    if (file == NULL || buffer == NULL || bytes_read == NULL) {
        return STATUS_INVALID;
    }

    /* Check if file is open for reading */
    if (!(file->mode & FAT_MODE_READ)) {
        return STATUS_INVALID;
    }

    /* Initialize bytes read */
    *bytes_read = 0;

    /* Check if we've reached the end of file */
    if (file->position >= file->info.size) {
        return STATUS_SUCCESS;
    }

    /* Calculate how many bytes we can read */
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
            int32_t status = fat_read_sector(file->sector, file->sector_buffer);
            if (status != STATUS_SUCCESS) {
                return status;
            }
        }

        /* Calculate how many bytes we can read from current sector */
        uint32_t bytes_in_sector = FAT_SECTOR_SIZE - file->offset;
        uint32_t bytes_to_copy = (bytes_remaining < bytes_in_sector) ? 
                                bytes_remaining : bytes_in_sector;

        /* Copy data */
        memcpy(ptr, &file->sector_buffer[file->offset], bytes_to_copy);
        ptr += bytes_to_copy;
        bytes_remaining -= bytes_to_copy;

        /* Update position */
        file->position += bytes_to_copy;
        file->offset += bytes_to_copy;

        /* Move to next sector if needed */
        if (file->offset >= FAT_SECTOR_SIZE) {
            file->offset = 0;
            file->sector++;

            /* Move to next cluster if needed */
            if (file->sector >= fat_ctx.config.sectors_per_cluster) {
                uint32_t next_cluster;
                int32_t status = fat_read_fat_entry(file->cluster, &next_cluster);
                if (status != STATUS_SUCCESS) {
                    return status;
                }

                if (next_cluster >= FAT_EOC(fat_type)) {
                    return STATUS_INVALID;
                }

                file->cluster = next_cluster;
                file->sector = fat_cluster_to_sector(file->cluster);
            }
        }
    }

    *bytes_read = bytes_to_read;
    return STATUS_SUCCESS;
}

int32_t fat_write(fat_file_t *file, const void *buffer, uint32_t size, uint32_t *bytes_written)
{
    if (file == NULL || buffer == NULL || bytes_written == NULL) {
        return STATUS_INVALID;
    }

    /* Check if file is open for writing */
    if (!(file->mode & FAT_MODE_WRITE)) {
        return STATUS_INVALID;
    }

    /* Initialize bytes written */
    *bytes_written = 0;

    /* Write data */
    const uint8_t *ptr = (const uint8_t *)buffer;
    uint32_t bytes_remaining = size;

    while (bytes_remaining > 0) {
        /* Read sector if needed */
        if (file->offset == 0) {
            int32_t status = fat_read_sector(file->sector, file->sector_buffer);
            if (status != STATUS_SUCCESS) {
                return status;
            }
        }

        /* Calculate how many bytes we can write to current sector */
        uint32_t bytes_in_sector = FAT_SECTOR_SIZE - file->offset;
        uint32_t bytes_to_copy = (bytes_remaining < bytes_in_sector) ? 
                                bytes_remaining : bytes_in_sector;

        /* Copy data */
        memcpy(&file->sector_buffer[file->offset], ptr, bytes_to_copy);
        ptr += bytes_to_copy;
        bytes_remaining -= bytes_to_copy;

        /* Update position */
        file->position += bytes_to_copy;
        file->offset += bytes_to_copy;

        /* Write sector if it's full */
        if (file->offset >= FAT_SECTOR_SIZE) {
            int32_t status = fat_write_sector(file->sector, file->sector_buffer);
            if (status != STATUS_SUCCESS) {
                return status;
            }

            file->offset = 0;
            file->sector++;

            /* Move to next cluster if needed */
            if (file->sector >= fat_ctx.config.sectors_per_cluster) {
                uint32_t next_cluster;
                int32_t status = fat_read_fat_entry(file->cluster, &next_cluster);
                if (status != STATUS_SUCCESS) {
                    return status;
                }

                if (next_cluster >= FAT_EOC(fat_type)) {
                    /* Allocate new cluster */
                    status = fat_find_free_cluster(&next_cluster);
                    if (status != STATUS_SUCCESS) {
                        return status;
                    }

                    /* Update FAT */
                    status = fat_write_fat_entry(file->cluster, next_cluster);
                    if (status != STATUS_SUCCESS) {
                        return status;
                    }

                    /* Clear new cluster */
                    memset(file->sector_buffer, 0, FAT_SECTOR_SIZE);
                    for (uint32_t i = 0; i < fat_ctx.config.sectors_per_cluster; i++) {
                        status = fat_write_sector(fat_cluster_to_sector(next_cluster) + i, 
                                                file->sector_buffer);
                        if (status != STATUS_SUCCESS) {
                            return status;
                        }
                    }
                }

                file->cluster = next_cluster;
                file->sector = fat_cluster_to_sector(file->cluster);
            }
        }
    }

    /* Write last sector if needed */
    if (file->offset > 0) {
        int32_t status = fat_write_sector(file->sector, file->sector_buffer);
        if (status != STATUS_SUCCESS) {
            return status;
        }
    }

    /* Update file size */
    if (file->position > file->info.size) {
        file->info.size = file->position;
        file->modified = true;
    }

    *bytes_written = size;
    return STATUS_SUCCESS;
}

int32_t fat_seek(fat_file_t *file, int32_t offset, int32_t origin)
{
    if (file == NULL) {
        return STATUS_INVALID;
    }

    /* Calculate new position */
    uint32_t new_position;
    switch (origin) {
        case SEEK_SET:
            new_position = offset;
            break;
        case SEEK_CUR:
            new_position = file->position + offset;
            break;
        case SEEK_END:
            new_position = file->info.size + offset;
            break;
        default:
            return STATUS_INVALID;
    }

    /* Check if new position is valid */
    if (new_position > file->info.size) {
        return STATUS_INVALID;
    }

    /* Calculate new cluster and sector */
    uint32_t new_cluster = file->info.cluster;
    uint32_t new_sector = fat_cluster_to_sector(new_cluster);
    uint32_t new_offset = new_position % FAT_SECTOR_SIZE;

    /* Move to target position */
    while (new_position >= FAT_SECTOR_SIZE) {
        uint32_t next_cluster;
        int32_t status = fat_read_fat_entry(new_cluster, &next_cluster);
        if (status != STATUS_SUCCESS) {
            return status;
        }

        if (next_cluster >= FAT_EOC(fat_type)) {
            return STATUS_INVALID;
        }

        new_cluster = next_cluster;
        new_sector = fat_cluster_to_sector(new_cluster);
        new_position -= FAT_SECTOR_SIZE;
    }

    /* Update file handle */
    file->position = new_position;
    file->cluster = new_cluster;
    file->sector = new_sector;
    file->offset = new_offset;

    return STATUS_SUCCESS;
}

int32_t fat_stat(const char *path, fat_dir_entry_t *info)
{
    if (path == NULL || info == NULL) {
        return STATUS_INVALID;
    }

    return fat_find_file(path, info);
}

int32_t fat_unlink(const char *path)
{
    if (path == NULL) {
        return STATUS_INVALID;
    }

    /* Find file entry */
    fat_dir_entry_t entry;
    int32_t status = fat_find_file(path, &entry);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    /* Check if it's a directory */
    if (entry.attributes & FAT_ATTR_DIRECTORY) {
        return STATUS_INVALID;
    }

    /* Free clusters */
    uint32_t cluster = (entry.first_cluster_hi << 16) | entry.first_cluster_lo;
    while (cluster < FAT_EOC(fat_type)) {
        uint32_t next_cluster;
        status = fat_read_fat_entry(cluster, &next_cluster);
        if (status != STATUS_SUCCESS) {
            return status;
        }

        status = fat_free_cluster(cluster);
        if (status != STATUS_SUCCESS) {
            return status;
        }

        cluster = next_cluster;
    }

    /* Mark entry as deleted */
    entry.name[0] = FAT_DIR_DELETED;
    return fat_write_dir_entry(&entry);
}

int32_t fat_mkdir(const char *path)
{
    if (path == NULL) {
        return STATUS_INVALID;
    }

    /* Check if directory already exists */
    fat_dir_entry_t entry;
    int32_t status = fat_find_file(path, &entry);
    if (status == STATUS_SUCCESS) {
        return STATUS_EXISTS;
    }

    /* Create directory entry */
    memset(&entry, 0, sizeof(fat_dir_entry_t));
    entry.attributes = FAT_ATTR_DIRECTORY;
    entry.first_cluster_hi = 0;
    entry.first_cluster_lo = 0;
    entry.file_size = 0;
    entry.last_write_date = fat_get_date();
    entry.last_write_time = fat_get_time();

    /* Convert path to short name */
    char short_name[12];
    status = fat_convert_to_short_name(path, short_name);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    memcpy(entry.name, short_name, 11);

    /* Write entry */
    status = fat_write_dir_entry(&entry);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    /* Allocate cluster for directory */
    uint32_t cluster;
    status = fat_alloc_cluster();
    if (status != STATUS_SUCCESS) {
        return status;
    }
    cluster = status;

    /* Update entry with cluster */
    entry.first_cluster_hi = (uint16_t)(cluster >> 16);
    entry.first_cluster_lo = (uint16_t)cluster;
    return fat_write_dir_entry(&entry);
}

int32_t fat_rmdir(const char *path)
{
    if (path == NULL) {
        return STATUS_INVALID;
    }

    /* Find directory entry */
    fat_dir_entry_t entry;
    int32_t status = fat_find_file(path, &entry);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    /* Check if it's a directory */
    if (!is_directory(&entry)) {
        return STATUS_INVALID;
    }

    /* Check if directory is empty */
    uint32_t cluster = (entry.first_cluster_hi << 16) | entry.first_cluster_lo;
    uint8_t sector_buffer[FAT_SECTOR_SIZE];
    bool is_empty = true;

    while (cluster < FAT_EOC(fat_type)) {
        uint32_t sector = fat_cluster_to_sector(cluster);
        for (uint32_t i = 0; i < fat_ctx.config.sectors_per_cluster; i++) {
            status = fat_read_sector(sector + i, sector_buffer);
            if (status != STATUS_SUCCESS) {
                return status;
            }

            fat_dir_entry_t *dir_entry = (fat_dir_entry_t *)sector_buffer;
            for (uint32_t j = 0; j < FAT_SECTOR_SIZE / sizeof(fat_dir_entry_t); j++) {
                if (dir_entry[j].name[0] != 0 && dir_entry[j].name[0] != FAT_DIR_DELETED) {
                    is_empty = false;
                    break;
                }
            }

            if (!is_empty) {
                break;
            }
        }

        if (!is_empty) {
            break;
        }

        uint32_t next_cluster;
        status = fat_read_fat_entry(cluster, &next_cluster);
        if (status != STATUS_SUCCESS) {
            return status;
        }

        cluster = next_cluster;
    }

    if (!is_empty) {
        return FAT_STATUS_NOT_EMPTY;
    }

    /* Free clusters */
    cluster = (entry.first_cluster_hi << 16) | entry.first_cluster_lo;
    while (cluster < FAT_EOC(fat_type)) {
        uint32_t next_cluster;
        status = fat_read_fat_entry(cluster, &next_cluster);
        if (status != STATUS_SUCCESS) {
            return status;
        }

        status = fat_free_cluster(cluster);
        if (status != STATUS_SUCCESS) {
            return status;
        }

        cluster = next_cluster;
    }

    /* Mark entry as deleted */
    entry.name[0] = FAT_DIR_DELETED;
    return fat_write_dir_entry(&entry);
}

/*********************************************************************
 * Private Function Implementations
 *********************************************************************/

static int32_t read_sector(uint32_t sector, uint8_t *buffer)
{
    if (!buffer) {
        return STATUS_INVALID;
    }

    /* Kiểm tra cache */
    for (int i = 0; i < FAT_CACHE_SIZE; i++) {
        if (fat_ctx.cache[i].valid && fat_ctx.cache[i].sector == sector) {
            memcpy(buffer, fat_ctx.cache[i].data, FAT_SECTOR_SIZE);
            return STATUS_SUCCESS;
        }
    }

    /* Đọc từ HAL */
    int32_t status = hal_read_sector(sector, buffer);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    /* Cập nhật cache */
    for (int i = 0; i < FAT_CACHE_SIZE; i++) {
        if (!fat_ctx.cache[i].valid) {
            fat_ctx.cache[i].valid = true;
            fat_ctx.cache[i].sector = sector;
            fat_ctx.cache[i].dirty = false;
            memcpy(fat_ctx.cache[i].data, buffer, FAT_SECTOR_SIZE);
            break;
        }
    }

    return STATUS_SUCCESS;
}

static uint32_t fat_cluster_to_sector(uint32_t cluster)
{
    if (cluster < 2) {
        return 0;
    }
    return ((cluster - 2) * fat_ctx.config.sectors_per_cluster) + fat_ctx.config.first_data_sector;
}

static uint16_t fat_get_time(void)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    
    uint16_t time = 0;
    time |= (tm->tm_sec / 2) & 0x1F;     /* Seconds/2 (0-29) */
    time |= (tm->tm_min & 0x3F) << 5;    /* Minutes (0-59) */
    time |= (tm->tm_hour & 0x1F) << 11;  /* Hours (0-23) */
    
    return time;
}

static uint16_t fat_get_date(void)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    
    uint16_t date = 0;
    date |= (tm->tm_mday & 0x1F);        /* Day (1-31) */
    date |= ((tm->tm_mon + 1) & 0x0F) << 5;  /* Month (1-12) */
    date |= ((tm->tm_year - 80) & 0x7F) << 9;  /* Year (0-127 + 1980) */
    
    return date;
}

static int32_t is_directory(const fat_dir_entry_t *entry)
{
    return (entry->attributes & FAT_ATTR_DIRECTORY) != 0;
}

static int32_t list_directory(const char *path)
{
    if (path == NULL) {
        return STATUS_INVALID;
    }

    /* Find directory entry */
    fat_dir_entry_t entry;
    int32_t status = fat_find_file(path, &entry);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    /* Check if it's a directory */
    if (!is_directory(&entry)) {
        return STATUS_INVALID;
    }

    /* Get directory cluster */
    uint32_t cluster = (entry.first_cluster_hi << 16) | entry.first_cluster_lo;
    uint8_t sector_buffer[FAT_SECTOR_SIZE];

    /* Read directory entries */
    while (cluster < FAT_EOC(fat_type)) {
        uint32_t sector = fat_cluster_to_sector(cluster);
        for (uint32_t i = 0; i < fat_ctx.config.sectors_per_cluster; i++) {
            status = fat_read_sector(sector + i, sector_buffer);
            if (status != STATUS_SUCCESS) {
                return status;
            }

            fat_dir_entry_t *dir_entry = (fat_dir_entry_t *)sector_buffer;
            for (uint32_t j = 0; j < FAT_SECTOR_SIZE / sizeof(fat_dir_entry_t); j++) {
                if (dir_entry[j].name[0] == 0) {
                    /* End of directory */
                    return STATUS_SUCCESS;
                }

                if (dir_entry[j].name[0] == FAT_DIR_DELETED) {
                    continue;
                }

                /* Print entry */
                char name[13];
                fat_get_name(&dir_entry[j], name);
                printf("%s\n", name);
            }
        }

        /* Get next cluster */
        uint32_t next_cluster;
        status = fat_read_fat_entry(cluster, &next_cluster);
        if (status != STATUS_SUCCESS) {
            return status;
        }

        cluster = next_cluster;
    }

    return STATUS_SUCCESS;
}

static int32_t change_directory(const char *path)
{
    if (path == NULL) {
        return STATUS_INVALID;
    }

    /* Find directory entry */
    fat_dir_entry_t entry;
    int32_t status = fat_find_file(path, &entry);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    /* Check if it's a directory */
    if (!is_directory(&entry)) {
        return STATUS_INVALID;
    }

    /* Update path context */
    return update_path_context(path, (entry.first_cluster_hi << 16) | entry.first_cluster_lo);
}

static int32_t resolve_path(const char *path, char *resolved_path)
{
    if (path == NULL || resolved_path == NULL) {
        return STATUS_INVALID;
    }

    /* Initialize resolved path */
    resolved_path[0] = '\0';

    /* Handle absolute path */
    if (path[0] == '/') {
        strcpy(resolved_path, path);
        return STATUS_SUCCESS;
    }

    /* Handle relative path */
    strcpy(resolved_path, path_ctx.current_path);
    if (resolved_path[strlen(resolved_path) - 1] != '/') {
        strcat(resolved_path, "/");
    }
    strcat(resolved_path, path);

    return STATUS_SUCCESS;
}

static int32_t normalize_path(const char *path, char *normalized_path)
{
    if (path == NULL || normalized_path == NULL) {
        return STATUS_INVALID;
    }

    /* Initialize normalized path */
    normalized_path[0] = '\0';

    /* Handle root directory */
    if (strcmp(path, "/") == 0) {
        strcpy(normalized_path, "/");
        return STATUS_SUCCESS;
    }

    /* Remove trailing slash */
    size_t len = strlen(path);
    if (len > 0 && path[len - 1] == '/') {
        len--;
    }

    /* Copy path */
    strncpy(normalized_path, path, len);
    normalized_path[len] = '\0';

    return STATUS_SUCCESS;
}

static int32_t get_parent_path(const char *path, char *parent_path)
{
    if (path == NULL || parent_path == NULL) {
        return STATUS_INVALID;
    }

    /* Initialize parent path */
    parent_path[0] = '\0';

    /* Handle root directory */
    if (strcmp(path, "/") == 0) {
        strcpy(parent_path, "/");
        return STATUS_SUCCESS;
    }

    /* Find last slash */
    const char *last_slash = strrchr(path, '/');
    if (last_slash == NULL) {
        strcpy(parent_path, "/");
        return STATUS_SUCCESS;
    }

    /* Copy parent path */
    size_t len = last_slash - path;
    if (len == 0) {
        strcpy(parent_path, "/");
    } else {
        strncpy(parent_path, path, len);
        parent_path[len] = '\0';
    }

    return STATUS_SUCCESS;
}

static int32_t init_path_context(void)
{
    /* Initialize path context */
    strcpy(path_ctx.current_path, "/");
    path_ctx.current_cluster = fat_ctx.root_cluster;
    path_ctx.is_root = true;
    strcpy(path_ctx.parent_path, "/");
    path_ctx.parent_cluster = fat_ctx.root_cluster;

    return STATUS_SUCCESS;
}

static int32_t update_path_context(const char *new_path, uint32_t new_cluster)
{
    /* Update current path */
    strcpy(path_ctx.current_path, new_path);

    /* Update current cluster */
    path_ctx.current_cluster = new_cluster;

    /* Update root flag */
    path_ctx.is_root = (strcmp(new_path, "/") == 0);

    /* Update parent path and cluster */
    if (path_ctx.is_root) {
        strcpy(path_ctx.parent_path, "/");
        path_ctx.parent_cluster = fat_ctx.root_cluster;
    } else {
        char parent_path[256];
        int32_t status = get_parent_path(new_path, parent_path);
        if (status != STATUS_SUCCESS) {
            return status;
        }

        strcpy(path_ctx.parent_path, parent_path);

        fat_dir_entry_t entry;
        status = fat_find_file(parent_path, &entry);
        if (status != STATUS_SUCCESS) {
            return status;
        }

        path_ctx.parent_cluster = (entry.first_cluster_hi << 16) | entry.first_cluster_lo;
    }

    return STATUS_SUCCESS;
}

/*********************************************************************
 * UUID: 2b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/ 