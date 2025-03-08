/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Module FAT Driver cung cấp các hàm để đọc/ghi dữ liệu trên hệ thống
 *   tập tin FAT (File Allocation Table). Hỗ trợ các phiên bản FAT12,
 *   FAT16 và FAT32.
 *********************************************************************/

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdio.h>
#include <string.h>
#include "fat_driver.h"
#include "fat_driver_private.h"

/*********************************************************************
 * Private Variables
 *********************************************************************/

/* Module context */
static fat_context_t fat_ctx;

/* Sector cache */
static fat_cache_entry_t fat_cache[FAT_CACHE_SIZE];

/* Working buffer */
static uint8_t fat_buffer[FAT_BUFFER_SIZE];

/*********************************************************************
 * Private Function Implementations
 *********************************************************************/

static int32_t fat_read_sector(uint32_t sector, uint8_t *buffer)
{
    /* Kiểm tra cache */
    for (int i = 0; i < FAT_CACHE_SIZE; i++) {
        if (fat_cache[i].sector == sector) {
            memcpy(buffer, fat_cache[i].data, FAT_SECTOR_SIZE);
            fat_cache[i].access_count++;
            return FAT_SUCCESS;
        }
    }

    /* Đọc sector từ thiết bị */
    // TODO: Implement device read

    /* Cập nhật cache */
    int min_access = fat_cache[0].access_count;
    int min_index = 0;
    for (int i = 1; i < FAT_CACHE_SIZE; i++) {
        if (fat_cache[i].access_count < min_access) {
            min_access = fat_cache[i].access_count;
            min_index = i;
        }
    }

    if (fat_cache[min_index].dirty) {
        // TODO: Write back dirty sector
    }

    fat_cache[min_index].sector = sector;
    memcpy(fat_cache[min_index].data, buffer, FAT_SECTOR_SIZE);
    fat_cache[min_index].dirty = false;
    fat_cache[min_index].access_count = 1;

    return FAT_SUCCESS;
}

static int32_t fat_write_sector(uint32_t sector, const uint8_t *buffer)
{
    /* Kiểm tra cache */
    for (int i = 0; i < FAT_CACHE_SIZE; i++) {
        if (fat_cache[i].sector == sector) {
            memcpy(fat_cache[i].data, buffer, FAT_SECTOR_SIZE);
            fat_cache[i].dirty = true;
            fat_cache[i].access_count++;
            return FAT_SUCCESS;
        }
    }

    /* Cập nhật cache */
    int min_access = fat_cache[0].access_count;
    int min_index = 0;
    for (int i = 1; i < FAT_CACHE_SIZE; i++) {
        if (fat_cache[i].access_count < min_access) {
            min_access = fat_cache[i].access_count;
            min_index = i;
        }
    }

    if (fat_cache[min_index].dirty) {
        // TODO: Write back dirty sector
    }

    fat_cache[min_index].sector = sector;
    memcpy(fat_cache[min_index].data, buffer, FAT_SECTOR_SIZE);
    fat_cache[min_index].dirty = true;
    fat_cache[min_index].access_count = 1;

    return FAT_SUCCESS;
}

static int32_t fat_read_fat_entry(uint32_t cluster, uint32_t *next_cluster)
{
    uint32_t fat_offset;
    uint32_t fat_sector;
    uint32_t ent_offset;

    /* Tính offset trong bảng FAT */
    switch (fat_ctx.config.fat_type) {
        case FAT_TYPE_12:
            fat_offset = cluster + (cluster / 2);
            break;
        case FAT_TYPE_16:
            fat_offset = cluster * 2;
            break;
        case FAT_TYPE_32:
            fat_offset = cluster * 4;
            break;
        default:
            return FAT_INVALID;
    }

    /* Tính sector chứa entry */
    fat_sector = fat_ctx.config.reserved_sectors + (fat_offset / FAT_SECTOR_SIZE);
    ent_offset = fat_offset % FAT_SECTOR_SIZE;

    /* Đọc sector */
    if (fat_read_sector(fat_sector, fat_buffer) != FAT_SUCCESS) {
        return FAT_ERROR;
    }

    /* Đọc giá trị entry */
    switch (fat_ctx.config.fat_type) {
        case FAT_TYPE_12:
            if (cluster & 0x1) {
                *next_cluster = (*(uint16_t *)&fat_buffer[ent_offset] >> 4) & FAT12_MASK;
            } else {
                *next_cluster = *(uint16_t *)&fat_buffer[ent_offset] & FAT12_MASK;
            }
            break;
        case FAT_TYPE_16:
            *next_cluster = *(uint16_t *)&fat_buffer[ent_offset] & FAT16_MASK;
            break;
        case FAT_TYPE_32:
            *next_cluster = *(uint32_t *)&fat_buffer[ent_offset] & FAT32_MASK;
            break;
    }

    return FAT_SUCCESS;
}

static int32_t fat_write_fat_entry(uint32_t cluster, uint32_t next_cluster)
{
    uint32_t fat_offset;
    uint32_t fat_sector;
    uint32_t ent_offset;

    /* Tính offset trong bảng FAT */
    switch (fat_ctx.config.fat_type) {
        case FAT_TYPE_12:
            fat_offset = cluster + (cluster / 2);
            break;
        case FAT_TYPE_16:
            fat_offset = cluster * 2;
            break;
        case FAT_TYPE_32:
            fat_offset = cluster * 4;
            break;
        default:
            return FAT_INVALID;
    }

    /* Tính sector chứa entry */
    fat_sector = fat_ctx.config.reserved_sectors + (fat_offset / FAT_SECTOR_SIZE);
    ent_offset = fat_offset % FAT_SECTOR_SIZE;

    /* Đọc sector */
    if (fat_read_sector(fat_sector, fat_buffer) != FAT_SUCCESS) {
        return FAT_ERROR;
    }

    /* Ghi giá trị entry */
    switch (fat_ctx.config.fat_type) {
        case FAT_TYPE_12:
            if (cluster & 0x1) {
                *(uint16_t *)&fat_buffer[ent_offset] &= 0x000F;
                *(uint16_t *)&fat_buffer[ent_offset] |= (next_cluster << 4);
            } else {
                *(uint16_t *)&fat_buffer[ent_offset] &= 0xF000;
                *(uint16_t *)&fat_buffer[ent_offset] |= next_cluster;
            }
            break;
        case FAT_TYPE_16:
            *(uint16_t *)&fat_buffer[ent_offset] = next_cluster & FAT16_MASK;
            break;
        case FAT_TYPE_32:
            *(uint32_t *)&fat_buffer[ent_offset] = next_cluster & FAT32_MASK;
            break;
    }

    /* Ghi sector */
    if (fat_write_sector(fat_sector, fat_buffer) != FAT_SUCCESS) {
        return FAT_ERROR;
    }

    return FAT_SUCCESS;
}

static int32_t fat_find_free_cluster(uint32_t *cluster)
{
    uint32_t i;
    uint32_t next;

    for (i = 2; i < fat_ctx.config.total_clusters; i++) {
        if (fat_read_fat_entry(i, &next) != FAT_SUCCESS) {
            return FAT_ERROR;
        }
        if (next == FAT_FREE_CLUSTER) {
            *cluster = i;
            return FAT_SUCCESS;
        }
    }

    return FAT_DISK_FULL;
}

static int32_t fat_convert_to_short_name(const char *name, char *short_name)
{
    int i, j;
    int len = strlen(name);
    int dot_pos = -1;

    /* Tìm vị trí dấu chấm cuối cùng */
    for (i = len - 1; i >= 0; i--) {
        if (name[i] == '.') {
            dot_pos = i;
            break;
        }
    }

    /* Điền tên file */
    memset(short_name, ' ', 11);
    for (i = 0, j = 0; i < dot_pos && j < 8; i++) {
        if (name[i] != ' ' && name[i] != '.') {
            short_name[j++] = toupper(name[i]);
        }
    }

    /* Điền phần mở rộng */
    if (dot_pos >= 0) {
        for (i = dot_pos + 1, j = 8; i < len && j < 11; i++) {
            if (name[i] != ' ' && name[i] != '.') {
                short_name[j++] = toupper(name[i]);
            }
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

/*********************************************************************
 * Public Function Implementations
 *********************************************************************/

int32_t fat_init(const fat_boot_sector_t *boot_sector)
{
    if (boot_sector == NULL) {
        return FAT_INVALID;
    }

    /* Kiểm tra chữ ký boot sector */
    if (*(uint16_t *)&boot_sector->type.fat16.boot_signature != FAT_SIGNATURE_AA55) {
        return FAT_INVALID;
    }

    /* Xác định loại FAT */
    uint32_t total_sectors = (boot_sector->total_sectors_16 == 0) ?
                            boot_sector->total_sectors_32 : boot_sector->total_sectors_16;
    uint32_t fat_size = (boot_sector->fat_size_16 == 0) ?
                        boot_sector->type.fat32.fat_size_32 : boot_sector->fat_size_16;
    uint32_t root_dir_sectors = ((boot_sector->root_entries * 32) + 
                                (boot_sector->bytes_per_sector - 1)) /
                                boot_sector->bytes_per_sector;
    uint32_t data_sectors = total_sectors - (boot_sector->reserved_sectors +
                           (boot_sector->number_of_fats * fat_size) +
                           root_dir_sectors);
    uint32_t total_clusters = data_sectors / boot_sector->sectors_per_cluster;

    if (total_clusters < 4085) {
        fat_ctx.config.fat_type = FAT_TYPE_12;
    } else if (total_clusters < 65525) {
        fat_ctx.config.fat_type = FAT_TYPE_16;
    } else {
        fat_ctx.config.fat_type = FAT_TYPE_32;
    }

    /* Lưu cấu hình */
    fat_ctx.config.total_sectors = total_sectors;
    fat_ctx.config.fat_size = fat_size;
    fat_ctx.config.root_dir_sectors = root_dir_sectors;
    fat_ctx.config.first_data_sector = boot_sector->reserved_sectors +
                                      (boot_sector->number_of_fats * fat_size) +
                                      root_dir_sectors;
    fat_ctx.config.data_sectors = data_sectors;
    fat_ctx.config.total_clusters = total_clusters;

    /* Khởi tạo cache */
    memset(fat_cache, 0, sizeof(fat_cache));
    for (int i = 0; i < FAT_CACHE_SIZE; i++) {
        fat_cache[i].data = malloc(FAT_SECTOR_SIZE);
        if (fat_cache[i].data == NULL) {
            return FAT_NO_MEMORY;
        }
    }

    /* Cập nhật trạng thái */
    fat_ctx.state = FAT_STATE_INITIALIZED;

    return FAT_SUCCESS;
}

int32_t fat_open(const char *path, uint8_t mode, fat_file_t *file)
{
    if (path == NULL || file == NULL) {
        return FAT_INVALID;
    }

    if (fat_ctx.state != FAT_STATE_INITIALIZED) {
        return FAT_INVALID;
    }

    /* Tìm entry trong thư mục */
    fat_dir_entry_t entry;
    if (fat_find_directory_entry(fat_ctx.config.root_dir_sectors, path, &entry) != FAT_SUCCESS) {
        /* Nếu file không tồn tại và có mode tạo mới */
        if ((mode & FAT_MODE_CREATE) && !(mode & FAT_MODE_READ)) {
            /* Tạo entry mới */
            if (fat_create_directory_entry(fat_ctx.config.root_dir_sectors, path, 
                FAT_ATTR_ARCHIVE, &entry) != FAT_SUCCESS) {
                return FAT_ERROR;
            }
        } else {
            return FAT_NOT_FOUND;
        }
    }

    /* Kiểm tra quyền truy cập */
    if ((entry.attributes & FAT_ATTR_READ_ONLY) && (mode & FAT_MODE_WRITE)) {
        return FAT_READ_ONLY;
    }

    /* Khởi tạo file handle */
    memset(file, 0, sizeof(fat_file_t));
    file->info.attributes = entry.attributes;
    file->info.size = entry.file_size;
    file->info.cluster = (entry.first_cluster_hi << 16) | entry.first_cluster_lo;
    file->mode = mode;
    strncpy((char *)file->info.name, path, sizeof(file->info.name) - 1);

    /* Xử lý mode */
    if (mode & FAT_MODE_APPEND) {
        file->position = file->info.size;
    }
    if (mode & FAT_MODE_TRUNCATE) {
        file->info.size = 0;
        file->modified = true;
    }

    return FAT_SUCCESS;
}

int32_t fat_close(fat_file_t *file)
{
    if (file == NULL) {
        return FAT_INVALID;
    }

    if (fat_ctx.state != FAT_STATE_INITIALIZED) {
        return FAT_INVALID;
    }

    /* Cập nhật thông tin nếu file đã thay đổi */
    if (file->modified) {
        fat_dir_entry_t entry;
        if (fat_find_directory_entry(fat_ctx.config.root_dir_sectors, 
            (const char *)file->info.name, &entry) == FAT_SUCCESS) {
            
            /* Cập nhật kích thước */
            entry.file_size = file->info.size;
            
            /* Cập nhật cluster */
            entry.first_cluster_lo = file->info.cluster & 0xFFFF;
            entry.first_cluster_hi = (file->info.cluster >> 16) & 0xFFFF;
            
            /* Cập nhật thời gian */
            // TODO: Update time fields
            
            /* Ghi lại entry */
            uint32_t sector = fat_ctx.config.first_data_sector + 
                            ((file->info.cluster - 2) * fat_ctx.config.sectors_per_cluster);
            if (fat_write_sector(sector, (const uint8_t *)&entry) != FAT_SUCCESS) {
                return FAT_ERROR;
            }
        }
    }

    /* Xóa handle */
    memset(file, 0, sizeof(fat_file_t));

    return FAT_SUCCESS;
}

int32_t fat_read(fat_file_t *file, void *buffer, uint32_t size, uint32_t *bytes_read)
{
    if (file == NULL || buffer == NULL || bytes_read == NULL) {
        return FAT_INVALID;
    }

    if (fat_ctx.state != FAT_STATE_INITIALIZED) {
        return FAT_INVALID;
    }

    /* Kiểm tra quyền đọc */
    if (!(file->mode & FAT_MODE_READ)) {
        return FAT_INVALID;
    }

    /* Kiểm tra vị trí đọc */
    if (file->position >= file->info.size) {
        *bytes_read = 0;
        return FAT_EOF;
    }

    /* Giới hạn kích thước đọc */
    uint32_t remaining = file->info.size - file->position;
    uint32_t to_read = (size < remaining) ? size : remaining;
    *bytes_read = 0;

    /* Tính cluster và offset */
    uint32_t cluster = file->info.cluster;
    uint32_t offset = file->position;
    while (offset >= fat_ctx.config.sectors_per_cluster * FAT_SECTOR_SIZE) {
        /* Lấy cluster tiếp theo */
        if (fat_read_fat_entry(cluster, &cluster) != FAT_SUCCESS) {
            return FAT_ERROR;
        }
        if (cluster >= FAT12_EOC) {
            return FAT_ERROR;
        }
        offset -= fat_ctx.config.sectors_per_cluster * FAT_SECTOR_SIZE;
    }

    /* Đọc dữ liệu */
    uint32_t sector = fat_ctx.config.first_data_sector + 
                      ((cluster - 2) * fat_ctx.config.sectors_per_cluster);
    uint32_t sector_offset = offset % FAT_SECTOR_SIZE;
    uint32_t bytes_left = to_read;
    uint8_t *buf = (uint8_t *)buffer;

    while (bytes_left > 0) {
        /* Đọc sector */
        if (fat_read_sector(sector + (offset / FAT_SECTOR_SIZE), fat_buffer) != FAT_SUCCESS) {
            return FAT_ERROR;
        }

        /* Copy dữ liệu */
        uint32_t chunk = FAT_SECTOR_SIZE - sector_offset;
        if (chunk > bytes_left) {
            chunk = bytes_left;
        }
        memcpy(buf, fat_buffer + sector_offset, chunk);

        /* Cập nhật con trỏ */
        buf += chunk;
        bytes_left -= chunk;
        *bytes_read += chunk;
        file->position += chunk;
        offset += chunk;
        sector_offset = 0;

        /* Kiểm tra hết sector */
        if (offset >= fat_ctx.config.sectors_per_cluster * FAT_SECTOR_SIZE) {
            /* Lấy cluster tiếp theo */
            if (fat_read_fat_entry(cluster, &cluster) != FAT_SUCCESS) {
                return FAT_ERROR;
            }
            if (cluster >= FAT12_EOC) {
                break;
            }
            sector = fat_ctx.config.first_data_sector + 
                     ((cluster - 2) * fat_ctx.config.sectors_per_cluster);
            offset = 0;
        }
    }

    return FAT_SUCCESS;
}

int32_t fat_write(fat_file_t *file, const void *buffer, uint32_t size, uint32_t *bytes_written)
{
    if (file == NULL || buffer == NULL || bytes_written == NULL) {
        return FAT_INVALID;
    }

    if (fat_ctx.state != FAT_STATE_INITIALIZED) {
        return FAT_INVALID;
    }

    /* Kiểm tra quyền ghi */
    if (!(file->mode & FAT_MODE_WRITE)) {
        return FAT_INVALID;
    }

    /* Khởi tạo biến */
    *bytes_written = 0;
    uint32_t cluster = file->info.cluster;
    uint32_t offset = file->position;
    const uint8_t *buf = (const uint8_t *)buffer;
    uint32_t bytes_left = size;

    /* Tìm cluster hiện tại */
    while (offset >= fat_ctx.config.sectors_per_cluster * FAT_SECTOR_SIZE) {
        /* Lấy cluster tiếp theo */
        if (fat_read_fat_entry(cluster, &cluster) != FAT_SUCCESS) {
            return FAT_ERROR;
        }
        if (cluster >= FAT12_EOC) {
            /* Cần thêm cluster mới */
            uint32_t new_cluster;
            if (fat_find_free_cluster(&new_cluster) != FAT_SUCCESS) {
                return FAT_DISK_FULL;
            }
            if (fat_write_fat_entry(cluster, new_cluster) != FAT_SUCCESS) {
                return FAT_ERROR;
            }
            cluster = new_cluster;
        }
        offset -= fat_ctx.config.sectors_per_cluster * FAT_SECTOR_SIZE;
    }

    /* Ghi dữ liệu */
    while (bytes_left > 0) {
        /* Tính sector */
        uint32_t sector = fat_ctx.config.first_data_sector + 
                         ((cluster - 2) * fat_ctx.config.sectors_per_cluster) +
                         (offset / FAT_SECTOR_SIZE);
        uint32_t sector_offset = offset % FAT_SECTOR_SIZE;

        /* Đọc sector hiện tại nếu không ghi đầy sector */
        if (sector_offset > 0 || bytes_left < FAT_SECTOR_SIZE) {
            if (fat_read_sector(sector, fat_buffer) != FAT_SUCCESS) {
                return FAT_ERROR;
            }
        }

        /* Copy dữ liệu */
        uint32_t chunk = FAT_SECTOR_SIZE - sector_offset;
        if (chunk > bytes_left) {
            chunk = bytes_left;
        }
        memcpy(fat_buffer + sector_offset, buf, chunk);

        /* Ghi sector */
        if (fat_write_sector(sector, fat_buffer) != FAT_SUCCESS) {
            return FAT_ERROR;
        }

        /* Cập nhật con trỏ */
        buf += chunk;
        bytes_left -= chunk;
        *bytes_written += chunk;
        file->position += chunk;
        offset += chunk;

        /* Cập nhật kích thước file */
        if (file->position > file->info.size) {
            file->info.size = file->position;
        }

        /* Kiểm tra hết cluster */
        if (offset >= fat_ctx.config.sectors_per_cluster * FAT_SECTOR_SIZE) {
            /* Cần cluster mới */
            uint32_t new_cluster;
            if (fat_find_free_cluster(&new_cluster) != FAT_SUCCESS) {
                return FAT_DISK_FULL;
            }
            if (fat_write_fat_entry(cluster, new_cluster) != FAT_SUCCESS) {
                return FAT_ERROR;
            }
            cluster = new_cluster;
            offset = 0;
        }
    }

    /* Đánh dấu file đã thay đổi */
    file->modified = true;

    return FAT_SUCCESS;
}

int32_t fat_seek(fat_file_t *file, int32_t offset, int32_t origin)
{
    if (file == NULL) {
        return FAT_INVALID;
    }

    if (fat_ctx.state != FAT_STATE_INITIALIZED) {
        return FAT_INVALID;
    }

    // TODO: Implement file seek

    return FAT_SUCCESS;
}

int32_t fat_stat(const char *path, fat_file_info_t *info)
{
    if (path == NULL || info == NULL) {
        return FAT_INVALID;
    }

    if (fat_ctx.state != FAT_STATE_INITIALIZED) {
        return FAT_INVALID;
    }

    // TODO: Implement file stat

    return FAT_SUCCESS;
}

int32_t fat_unlink(const char *path)
{
    if (path == NULL) {
        return FAT_INVALID;
    }

    if (fat_ctx.state != FAT_STATE_INITIALIZED) {
        return FAT_INVALID;
    }

    // TODO: Implement file unlink

    return FAT_SUCCESS;
}

int32_t fat_mkdir(const char *path)
{
    if (path == NULL) {
        return FAT_INVALID;
    }

    if (fat_ctx.state != FAT_STATE_INITIALIZED) {
        return FAT_INVALID;
    }

    // TODO: Implement directory create

    return FAT_SUCCESS;
}

int32_t fat_rmdir(const char *path)
{
    if (path == NULL) {
        return FAT_INVALID;
    }

    if (fat_ctx.state != FAT_STATE_INITIALIZED) {
        return FAT_INVALID;
    }

    // TODO: Implement directory remove

    return FAT_SUCCESS;
}

/*********************************************************************
 * UUID: 4b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/
