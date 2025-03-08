/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   File header riêng của module FAT Driver, định nghĩa các hằng số,
 *   kiểu dữ liệu và hàm private chỉ sử dụng trong nội bộ module.
 *   KHÔNG sử dụng các định nghĩa này bên ngoài module.
 *********************************************************************/
#ifndef __FAT_DRIVER_PRIVATE_H
#define __FAT_DRIVER_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "fat_driver.h"
#include "../common/storage_driver.h"

/*********************************************************************
 * Macro Definitions
 *********************************************************************/

/* FAT Error Codes */
#define FAT_ERROR_INVALID_PARAMETER  (-1)
#define FAT_ERROR_NO_MEMORY         (-2)
#define FAT_ERROR_READ_FAILED       (-3)
#define FAT_ERROR_WRITE_FAILED      (-4)
#define FAT_ERROR_NOT_FOUND         (-5)
#define FAT_ERROR_NO_SPACE          (-6)

/* FAT Signature */
#define FAT_SIGNATURE_AA55    0xAA55  /* Chữ ký boot sector */

/* FAT Entry Values */
#define FAT12_EOC            0x0FF8  /* End of cluster FAT12 */
#define FAT16_EOC            0xFFF8  /* End of cluster FAT16 */
#define FAT32_EOC            0x0FFFFFF8  /* End of cluster FAT32 */
#define FAT_BAD_CLUSTER      0x0FFFFFF7  /* Bad cluster */
#define FAT_FREE_CLUSTER     0x00000000  /* Free cluster */

/* FAT Cluster Masks */
#define FAT12_MASK           0x0FFF  /* Mask 12 bit */
#define FAT16_MASK           0xFFFF  /* Mask 16 bit */
#define FAT32_MASK           0x0FFFFFFF  /* Mask 28 bit */

/* FAT Directory Entry */
#define FAT_DIR_ENTRY_SIZE   32      /* Kích thước entry */
#define FAT_DIR_NAME_LEN     8       /* Độ dài tên file */
#define FAT_DIR_EXT_LEN      3       /* Độ dài phần mở rộng */
#define FAT_DIR_DELETED      0xE5    /* Entry đã xóa */
#define FAT_DIR_EMPTY        0x00    /* Entry trống */

/* FAT Buffer Size */
#define FAT_BUFFER_SIZE      4096    /* Kích thước buffer */

/* FAT Cache Size */
#define FAT_CACHE_SIZE       16      /* Số lượng sector cache */

/* FAT Error Values */
#define FAT_INVALID_CLUSTER  0xFFFFFFFF  /* Cluster không hợp lệ */
#define FAT_INVALID_SECTOR   0xFFFFFFFF  /* Sector không hợp lệ */

/* FAT EOC Macro */
static inline uint32_t FAT_EOC(uint8_t type) {
    switch (type) {
        case FAT_TYPE_12:
            return FAT12_EOC;
        case FAT_TYPE_16:
            return FAT16_EOC;
        case FAT_TYPE_32:
            return FAT32_EOC;
        default:
            return FAT32_EOC;
    }
}

/*********************************************************************
 * Type Definitions
 *********************************************************************/

/* FAT Module State */
typedef enum {
    FAT_STATE_UNINITIALIZED = 0,  /* Chưa khởi tạo */
    FAT_STATE_INITIALIZED,         /* Đã khởi tạo */
    FAT_STATE_ERROR               /* Lỗi */
} fat_state_t;

/* FAT Module Configuration */
typedef struct {
    uint8_t fat_type;            /* Loại FAT */
    uint32_t total_sectors;      /* Tổng số sector */
    uint32_t fat_size;          /* Kích thước FAT */
    uint32_t root_dir_sectors;  /* Số sector thư mục gốc */
    uint32_t first_data_sector; /* Sector dữ liệu đầu tiên */
    uint32_t data_sectors;      /* Số sector dữ liệu */
    uint32_t total_clusters;    /* Tổng số cluster */
    uint32_t sectors_per_cluster; /* Số sector mỗi cluster */
    uint32_t reserved_sectors;   /* Số sector dự trữ */
} fat_config_t;

/* FAT Module Context */
typedef struct {
    fat_state_t state;          /* Trạng thái module */
    fat_config_t config;        /* Cấu hình */
    storage_driver_t *storage;     /* Storage driver đang sử dụng */
    fat_boot_sector_t boot;        /* Boot sector */
    uint32_t fat_start;            /* Sector bắt đầu của FAT */
    uint32_t root_start;           /* Sector bắt đầu của root directory */
    uint32_t data_start;           /* Sector bắt đầu của data area */
    uint32_t total_sectors;        /* Tổng số sector */
    uint32_t sectors_per_fat;      /* Số sector mỗi FAT */
    uint32_t root_sectors;         /* Số sector của root directory */
    bool mounted;                 /* Đã mount chưa */

    #if FAT_ENABLE_CACHE
    struct {
        uint32_t sector;          /* Sector number */
        uint8_t data[FAT_SECTOR_SIZE]; /* Sector data */
        bool valid;               /* Cache valid? */
        bool dirty;               /* Cache modified? */
    } cache[FAT_CACHE_SIZE];
    #endif
} fat_context_t;

/* FAT Cache Entry */
typedef struct {
    uint32_t sector;           /* Số hiệu sector */
    uint8_t *data;            /* Dữ liệu sector */
    bool dirty;               /* Đã thay đổi */
    uint32_t access_count;    /* Số lần truy cập */
} fat_cache_entry_t;

/*********************************************************************
 * Private Function Prototypes
 *********************************************************************/

/**
 * @brief Đọc một sector từ thiết bị
 * 
 * @param sector Số hiệu sector
 * @param buffer Buffer lưu dữ liệu
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_read_sector(uint32_t sector, uint8_t *buffer);

/**
 * @brief Ghi một sector xuống thiết bị
 * 
 * @param sector Số hiệu sector
 * @param buffer Buffer chứa dữ liệu
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_write_sector(uint32_t sector, const uint8_t *buffer);

/**
 * @brief Đọc một entry từ bảng FAT
 * 
 * @param cluster Số hiệu cluster
 * @param next_cluster Con trỏ đến cluster tiếp theo
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_read_fat_entry(uint32_t cluster, uint32_t *next_cluster);

/**
 * @brief Ghi một entry vào bảng FAT
 * 
 * @param cluster Số hiệu cluster
 * @param next_cluster Giá trị cluster tiếp theo
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_write_fat_entry(uint32_t cluster, uint32_t next_cluster);

/**
 * @brief Tìm cluster trống trong bảng FAT
 * 
 * @param cluster Con trỏ đến cluster tìm được
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_find_free_cluster(uint32_t *cluster);

/**
 * @brief Chuyển đổi tên file sang định dạng 8.3
 * 
 * @param name Tên file gốc
 * @param short_name Buffer lưu tên file 8.3
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_convert_to_short_name(const char *name, char *short_name);

/**
 * @brief Tính checksum cho tên file 8.3
 * 
 * @param short_name Tên file 8.3
 * @return Giá trị checksum
 */
uint8_t fat_calculate_short_name_checksum(const char *short_name);

/**
 * @brief Tìm tập tin/thư mục theo đường dẫn
 * 
 * @param path Đường dẫn cần tìm
 * @param entry Con trỏ đến entry tìm được
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_find_file(const char *path, fat_dir_entry_t *entry);

/**
 * @brief Tạo tập tin mới
 * 
 * @param path Đường dẫn tập tin
 * @param entry Con trỏ đến entry được tạo
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_create_file(const char *path, fat_dir_entry_t *entry);

/**
 * @brief Ghi entry vào thư mục
 * 
 * @param entry Con trỏ đến entry cần ghi
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_write_dir_entry(const fat_dir_entry_t *entry);

/**
 * @brief Cấp phát cluster mới
 * 
 * @return Số hiệu cluster được cấp phát, 0 nếu thất bại
 */
uint32_t fat_alloc_cluster(void);

/**
 * @brief Giải phóng cluster
 * 
 * @param cluster Số hiệu cluster cần giải phóng
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_free_cluster(uint32_t cluster);

/**
 * @brief Tính sector đầu tiên của cluster
 * 
 * @param cluster Số hiệu cluster
 * @return Số hiệu sector đầu tiên
 */
uint32_t get_first_sector(uint32_t cluster);

/**
 * @brief Lấy tên file từ entry
 */
void fat_get_name(const fat_dir_entry_t *entry, char *name);

/**
 * @brief Chuyển cluster thành sector
 */
uint32_t fat_cluster_to_sector(uint32_t cluster);

/**
 * @brief Lấy thời gian hiện tại
 */
uint16_t fat_get_time(void);

/**
 * @brief Lấy ngày hiện tại
 */
uint16_t fat_get_date(void);

#ifdef __cplusplus
}
#endif

#endif /* __FAT_DRIVER_PRIVATE_H */

/*********************************************************************
 * UUID: 1c9d2f1b-4c4a-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/ 