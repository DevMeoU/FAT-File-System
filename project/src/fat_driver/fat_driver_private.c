/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   File triển khai các hàm private của module FAT Driver.
 *   Các hàm này chỉ được sử dụng trong nội bộ module.
 *********************************************************************/

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "fat_driver_private.h"
#include "../ip_driver/ip_driver.h"

/*********************************************************************
 * Private Variables
 *********************************************************************/
static fat_context_t fat_context;
static fat_cache_entry_t fat_cache[FAT_CACHE_SIZE];
static uint8_t cache_data[FAT_CACHE_SIZE][FAT_SECTOR_SIZE];

/*********************************************************************
 * Private Function Implementations
 *********************************************************************/

static int32_t fat_read_sector(uint32_t sector, uint8_t *buffer)
{
    /* Check parameters */
    if (buffer == NULL) {
        return FAT_INVALID;
    }

    /* Check cache first */
    for (int i = 0; i < FAT_CACHE_SIZE; i++) {
        if (fat_cache[i].sector == sector) {
            memcpy(buffer, fat_cache[i].data, FAT_SECTOR_SIZE);
            fat_cache[i].access_count++;
            return FAT_SUCCESS;
        }
    }

    /* Read from device */
    if (ip_read_sector(sector, buffer) != IP_SUCCESS) {
        return FAT_ERROR;
    }

    /* Add to cache */
    int min_access = fat_cache[0].access_count;
    int min_index = 0;
    for (int i = 1; i < FAT_CACHE_SIZE; i++) {
        if (fat_cache[i].access_count < min_access) {
            min_access = fat_cache[i].access_count;
            min_index = i;
        }
    }

    /* Write back dirty cache entry */
    if (fat_cache[min_index].dirty) {
        if (ip_write_sector(fat_cache[min_index].sector, fat_cache[min_index].data) != IP_SUCCESS) {
            return FAT_ERROR;
        }
    }

    fat_cache[min_index].sector = sector;
    memcpy(fat_cache[min_index].data, buffer, FAT_SECTOR_SIZE);
    fat_cache[min_index].dirty = false;
    fat_cache[min_index].access_count = 1;

    return FAT_SUCCESS;
}

static int32_t fat_write_sector(uint32_t sector, const uint8_t *buffer)
{
    /* Check parameters */
    if (buffer == NULL) {
        return FAT_INVALID;
    }

    /* Update cache if present */
    for (int i = 0; i < FAT_CACHE_SIZE; i++) {
        if (fat_cache[i].sector == sector) {
            memcpy(fat_cache[i].data, buffer, FAT_SECTOR_SIZE);
            fat_cache[i].dirty = true;
            fat_cache[i].access_count++;
            return FAT_SUCCESS;
        }
    }

    /* Add to cache */
    int min_access = fat_cache[0].access_count;
    int min_index = 0;
    for (int i = 1; i < FAT_CACHE_SIZE; i++) {
        if (fat_cache[i].access_count < min_access) {
            min_access = fat_cache[i].access_count;
            min_index = i;
        }
    }

    /* Write back dirty cache entry */
    if (fat_cache[min_index].dirty) {
        if (ip_write_sector(fat_cache[min_index].sector, fat_cache[min_index].data) != IP_SUCCESS) {
            return FAT_ERROR;
        }
    }

    fat_cache[min_index].sector = sector;
    memcpy(fat_cache[min_index].data, buffer, FAT_SECTOR_SIZE);
    fat_cache[min_index].dirty = true;
    fat_cache[min_index].access_count = 1;

    return FAT_SUCCESS;
}

static int32_t fat_read_fat_entry(uint32_t cluster, uint32_t *next_cluster)
{
    /* Check parameters */
    if (next_cluster == NULL || cluster >= fat_context.config.total_clusters) {
        return FAT_INVALID;
    }

    uint32_t fat_offset;
    uint32_t fat_sector;
    uint32_t ent_offset;
    uint8_t sector_buffer[FAT_SECTOR_SIZE];

    switch (fat_context.config.fat_type) {
        case FAT_TYPE_12:
            fat_offset = cluster + (cluster / 2);
            fat_sector = fat_context.config.reserved_sectors + (fat_offset / FAT_SECTOR_SIZE);
            ent_offset = fat_offset % FAT_SECTOR_SIZE;

            if (fat_read_sector(fat_sector, sector_buffer) != FAT_SUCCESS) {
                return FAT_ERROR;
            }

            if (ent_offset == (FAT_SECTOR_SIZE - 1)) {
                /* Entry spans two sectors */
                uint8_t next_sector_buffer[FAT_SECTOR_SIZE];
                if (fat_read_sector(fat_sector + 1, next_sector_buffer) != FAT_SUCCESS) {
                    return FAT_ERROR;
                }

                if (cluster & 0x1) {
                    *next_cluster = (sector_buffer[ent_offset] >> 4) | (next_sector_buffer[0] << 4);
                } else {
                    *next_cluster = sector_buffer[ent_offset] | ((next_sector_buffer[0] & 0x0F) << 8);
                }
            } else {
                if (cluster & 0x1) {
                    *next_cluster = (sector_buffer[ent_offset] >> 4) | (sector_buffer[ent_offset + 1] << 4);
                } else {
                    *next_cluster = sector_buffer[ent_offset] | ((sector_buffer[ent_offset + 1] & 0x0F) << 8);
                }
            }
            *next_cluster &= FAT12_MASK;
            break;

        case FAT_TYPE_16:
            fat_offset = cluster * 2;
            fat_sector = fat_context.config.reserved_sectors + (fat_offset / FAT_SECTOR_SIZE);
            ent_offset = fat_offset % FAT_SECTOR_SIZE;

            if (fat_read_sector(fat_sector, sector_buffer) != FAT_SUCCESS) {
                return FAT_ERROR;
            }

            *next_cluster = *(uint16_t *)&sector_buffer[ent_offset];
            break;

        case FAT_TYPE_32:
            fat_offset = cluster * 4;
            fat_sector = fat_context.config.reserved_sectors + (fat_offset / FAT_SECTOR_SIZE);
            ent_offset = fat_offset % FAT_SECTOR_SIZE;

            if (fat_read_sector(fat_sector, sector_buffer) != FAT_SUCCESS) {
                return FAT_ERROR;
            }

            *next_cluster = *(uint32_t *)&sector_buffer[ent_offset] & FAT32_MASK;
            break;

        default:
            return FAT_ERROR;
    }

    return FAT_SUCCESS;
}

static int32_t fat_write_fat_entry(uint32_t cluster, uint32_t next_cluster)
{
    /* Check parameters */
    if (cluster >= fat_context.config.total_clusters) {
        return FAT_INVALID;
    }

    uint32_t fat_offset;
    uint32_t fat_sector;
    uint32_t ent_offset;
    uint8_t sector_buffer[FAT_SECTOR_SIZE];

    switch (fat_context.config.fat_type) {
        case FAT_TYPE_12:
            fat_offset = cluster + (cluster / 2);
            fat_sector = fat_context.config.reserved_sectors + (fat_offset / FAT_SECTOR_SIZE);
            ent_offset = fat_offset % FAT_SECTOR_SIZE;

            if (fat_read_sector(fat_sector, sector_buffer) != FAT_SUCCESS) {
                return FAT_ERROR;
            }

            if (ent_offset == (FAT_SECTOR_SIZE - 1)) {
                /* Entry spans two sectors */
                uint8_t next_sector_buffer[FAT_SECTOR_SIZE];
                if (fat_read_sector(fat_sector + 1, next_sector_buffer) != FAT_SUCCESS) {
                    return FAT_ERROR;
                }

                if (cluster & 0x1) {
                    sector_buffer[ent_offset] = (sector_buffer[ent_offset] & 0x0F) | ((next_cluster & 0x0F) << 4);
                    next_sector_buffer[0] = (next_cluster >> 4) & 0xFF;
                } else {
                    sector_buffer[ent_offset] = next_cluster & 0xFF;
                    next_sector_buffer[0] = (next_sector_buffer[0] & 0xF0) | ((next_cluster >> 8) & 0x0F);
                }

                if (fat_write_sector(fat_sector, sector_buffer) != FAT_SUCCESS) {
                    return FAT_ERROR;
                }
                if (fat_write_sector(fat_sector + 1, next_sector_buffer) != FAT_SUCCESS) {
                    return FAT_ERROR;
                }
            } else {
                if (cluster & 0x1) {
                    sector_buffer[ent_offset] = (sector_buffer[ent_offset] & 0x0F) | ((next_cluster & 0x0F) << 4);
                    sector_buffer[ent_offset + 1] = (next_cluster >> 4) & 0xFF;
                } else {
                    sector_buffer[ent_offset] = next_cluster & 0xFF;
                    sector_buffer[ent_offset + 1] = (sector_buffer[ent_offset + 1] & 0xF0) | ((next_cluster >> 8) & 0x0F);
                }

                if (fat_write_sector(fat_sector, sector_buffer) != FAT_SUCCESS) {
                    return FAT_ERROR;
                }
            }
            break;

        case FAT_TYPE_16:
            fat_offset = cluster * 2;
            fat_sector = fat_context.config.reserved_sectors + (fat_offset / FAT_SECTOR_SIZE);
            ent_offset = fat_offset % FAT_SECTOR_SIZE;

            if (fat_read_sector(fat_sector, sector_buffer) != FAT_SUCCESS) {
                return FAT_ERROR;
            }

            *(uint16_t *)&sector_buffer[ent_offset] = (uint16_t)next_cluster;

            if (fat_write_sector(fat_sector, sector_buffer) != FAT_SUCCESS) {
                return FAT_ERROR;
            }
            break;

        case FAT_TYPE_32:
            fat_offset = cluster * 4;
            fat_sector = fat_context.config.reserved_sectors + (fat_offset / FAT_SECTOR_SIZE);
            ent_offset = fat_offset % FAT_SECTOR_SIZE;

            if (fat_read_sector(fat_sector, sector_buffer) != FAT_SUCCESS) {
                return FAT_ERROR;
            }

            *(uint32_t *)&sector_buffer[ent_offset] = (*(uint32_t *)&sector_buffer[ent_offset] & ~FAT32_MASK) | (next_cluster & FAT32_MASK);

            if (fat_write_sector(fat_sector, sector_buffer) != FAT_SUCCESS) {
                return FAT_ERROR;
            }
            break;

        default:
            return FAT_ERROR;
    }

    return FAT_SUCCESS;
}

static int32_t fat_find_free_cluster(uint32_t *cluster)
{
    /* Check parameters */
    if (cluster == NULL) {
        return FAT_INVALID;
    }

    uint32_t current_cluster = 2;  /* First valid cluster */
    uint32_t next_cluster;

    while (current_cluster < fat_context.config.total_clusters) {
        if (fat_read_fat_entry(current_cluster, &next_cluster) != FAT_SUCCESS) {
            return FAT_ERROR;
        }

        if (next_cluster == FAT_FREE_CLUSTER) {
            *cluster = current_cluster;
            return FAT_SUCCESS;
        }

        current_cluster++;
    }

    return FAT_DISK_FULL;
}

static int32_t fat_convert_to_short_name(const char *name, char *short_name)
{
    /* Check parameters */
    if (name == NULL || short_name == NULL) {
        return FAT_INVALID;
    }

    /* Clear short name buffer */
    memset(short_name, ' ', 11);

    /* Find extension */
    const char *ext = strrchr(name, '.');
    size_t name_len = (ext != NULL) ? (ext - name) : strlen(name);
    size_t ext_len = (ext != NULL) ? strlen(ext + 1) : 0;

    /* Check lengths */
    if (name_len > FAT_DIR_NAME_LEN || ext_len > FAT_DIR_EXT_LEN) {
        return FAT_INVALID_NAME;
    }

    /* Copy name */
    for (size_t i = 0; i < name_len && i < FAT_DIR_NAME_LEN; i++) {
        short_name[i] = toupper(name[i]);
    }

    /* Copy extension */
    if (ext != NULL) {
        for (size_t i = 0; i < ext_len && i < FAT_DIR_EXT_LEN; i++) {
            short_name[FAT_DIR_NAME_LEN + i] = toupper(ext[i + 1]);
        }
    }

    return FAT_SUCCESS;
}

static uint8_t fat_calculate_short_name_checksum(const char *short_name)
{
    uint8_t sum = 0;
    for (int i = 0; i < 11; i++) {
        sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + short_name[i];
    }
    return sum;
}

static int32_t fat_find_file(const char *path, fat_dir_entry_t *entry)
{
    if (path == NULL || entry == NULL) {
        return FAT_INVALID;
    }

    /* Parse path components */
    char component[256];
    const char *p = path;
    uint32_t cluster = fat_context.config.root_dir_sectors;

    while (*p) {
        /* Get next component */
        char *c = component;
        while (*p && *p != '/') {
            *c++ = *p++;
        }
        *c = '\0';
        if (*p == '/') p++;

        /* Skip empty components */
        if (component[0] == '\0') {
            continue;
        }

        /* Convert to short name */
        char short_name[11];
        if (fat_convert_to_short_name(component, short_name) != FAT_SUCCESS) {
            return FAT_INVALID_NAME;
        }

        /* Search in current directory */
        uint32_t sector = get_first_sector(cluster);
        uint32_t sector_count = fat_context.config.sectors_per_cluster;
        bool found = false;

        for (uint32_t i = 0; i < sector_count && !found; i++) {
            uint8_t sector_buffer[FAT_SECTOR_SIZE];
            if (fat_read_sector(sector + i, sector_buffer) != FAT_SUCCESS) {
                return FAT_ERROR;
            }

            fat_dir_entry_t *dir = (fat_dir_entry_t *)sector_buffer;
            for (int j = 0; j < FAT_SECTOR_SIZE/sizeof(fat_dir_entry_t); j++) {
                if (dir[j].name[0] == FAT_DIR_EMPTY) {
                    break;  /* End of directory */
                }
                if (dir[j].name[0] == FAT_DIR_DELETED) {
                    continue;
                }
                if (memcmp(dir[j].name, short_name, 11) == 0) {
                    memcpy(entry, &dir[j], sizeof(fat_dir_entry_t));
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            return FAT_NOT_FOUND;
        }

        /* Move to next directory */
        if (*p) {
            if (!(entry->attributes & FAT_ATTR_DIRECTORY)) {
                return FAT_INVALID_PATH;
            }
            cluster = (entry->first_cluster_hi << 16) | entry->first_cluster_lo;
        }
    }

    return FAT_SUCCESS;
}

static int32_t fat_create_file(const char *path, fat_dir_entry_t *entry)
{
    if (path == NULL || entry == NULL) {
        return FAT_INVALID;
    }

    /* Get parent directory path */
    char parent_path[256];
    const char *name = strrchr(path, '/');
    if (name == NULL) {
        parent_path[0] = '\0';
        name = path;
    } else {
        size_t len = name - path;
        strncpy(parent_path, path, len);
        parent_path[len] = '\0';
        name++;
    }

    /* Find parent directory */
    fat_dir_entry_t parent_entry;
    int32_t status;
    uint32_t parent_cluster;

    if (parent_path[0] == '\0') {
        parent_cluster = fat_context.config.root_dir_sectors;
    } else {
        status = fat_find_file(parent_path, &parent_entry);
        if (status != FAT_SUCCESS) {
            return status;
        }
        if (!(parent_entry.attributes & FAT_ATTR_DIRECTORY)) {
            return FAT_INVALID_PATH;
        }
        parent_cluster = (parent_entry.first_cluster_hi << 16) | parent_entry.first_cluster_lo;
    }

    /* Convert name to short format */
    char short_name[11];
    if (fat_convert_to_short_name(name, short_name) != FAT_SUCCESS) {
        return FAT_INVALID_NAME;
    }

    /* Find free entry in parent directory */
    uint32_t sector = get_first_sector(parent_cluster);
    uint32_t sector_count = fat_context.config.sectors_per_cluster;
    bool found = false;

    for (uint32_t i = 0; i < sector_count && !found; i++) {
        uint8_t sector_buffer[FAT_SECTOR_SIZE];
        if (fat_read_sector(sector + i, sector_buffer) != FAT_SUCCESS) {
            return FAT_ERROR;
        }

        fat_dir_entry_t *dir = (fat_dir_entry_t *)sector_buffer;
        for (int j = 0; j < FAT_SECTOR_SIZE/sizeof(fat_dir_entry_t); j++) {
            if (dir[j].name[0] == FAT_DIR_EMPTY || 
                dir[j].name[0] == FAT_DIR_DELETED) {
                /* Initialize new entry */
                memset(&dir[j], 0, sizeof(fat_dir_entry_t));
                memcpy(dir[j].name, short_name, 11);
                dir[j].attributes = 0;
                dir[j].creation_time = 0;  /* TODO: Set current time */
                dir[j].creation_date = 0;  /* TODO: Set current date */
                dir[j].last_access_date = 0;
                dir[j].last_write_time = 0;
                dir[j].last_write_date = 0;
                dir[j].first_cluster_hi = 0;
                dir[j].first_cluster_lo = 0;
                dir[j].file_size = 0;

                /* Write sector back */
                if (fat_write_sector(sector + i, sector_buffer) != FAT_SUCCESS) {
                    return FAT_ERROR;
                }

                /* Return entry */
                memcpy(entry, &dir[j], sizeof(fat_dir_entry_t));
                found = true;
                break;
            }
        }
    }

    if (!found) {
        return FAT_ROOT_FULL;
    }

    return FAT_SUCCESS;
}

static int32_t fat_write_dir_entry(const fat_dir_entry_t *entry)
{
    if (entry == NULL) {
        return FAT_INVALID;
    }

    /* Find entry in directory */
    uint32_t cluster = fat_context.config.root_dir_sectors;
    uint32_t sector = get_first_sector(cluster);
    uint32_t sector_count = fat_context.config.sectors_per_cluster;
    bool found = false;

    for (uint32_t i = 0; i < sector_count && !found; i++) {
        uint8_t sector_buffer[FAT_SECTOR_SIZE];
        if (fat_read_sector(sector + i, sector_buffer) != FAT_SUCCESS) {
            return FAT_ERROR;
        }

        fat_dir_entry_t *dir = (fat_dir_entry_t *)sector_buffer;
        for (int j = 0; j < FAT_SECTOR_SIZE/sizeof(fat_dir_entry_t); j++) {
            if (dir[j].name[0] == FAT_DIR_EMPTY) {
                break;  /* End of directory */
            }
            if (memcmp(dir[j].name, entry->name, 11) == 0) {
                /* Update entry */
                memcpy(&dir[j], entry, sizeof(fat_dir_entry_t));

                /* Write sector back */
                if (fat_write_sector(sector + i, sector_buffer) != FAT_SUCCESS) {
                    return FAT_ERROR;
                }

                found = true;
                break;
            }
        }
    }

    if (!found) {
        return FAT_NOT_FOUND;
    }

    return FAT_SUCCESS;
}

static uint32_t fat_alloc_cluster(void)
{
    uint32_t cluster;
    if (fat_find_free_cluster(&cluster) != FAT_SUCCESS) {
        return 0;
    }

    /* Mark cluster as end of chain */
    uint32_t eoc;
    switch (fat_context.config.fat_type) {
        case FAT_TYPE_12:
            eoc = FAT12_EOC;
            break;
        case FAT_TYPE_16:
            eoc = FAT16_EOC;
            break;
        case FAT_TYPE_32:
            eoc = FAT32_EOC;
            break;
        default:
            return 0;
    }

    if (fat_write_fat_entry(cluster, eoc) != FAT_SUCCESS) {
        return 0;
    }

    return cluster;
}

static int32_t fat_free_cluster(uint32_t cluster)
{
    if (cluster < 2 || cluster >= fat_context.config.total_clusters) {
        return FAT_INVALID;
    }

    return fat_write_fat_entry(cluster, FAT_FREE_CLUSTER);
}

/*********************************************************************
 * UUID: 2b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/ 