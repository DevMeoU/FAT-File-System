/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Module Middleware cung cấp lớp trung gian giữa tầng ứng dụng và
 *   tầng driver, xử lý các yêu cầu từ ứng dụng và chuyển đổi thành
 *   các lệnh phù hợp cho driver.
 *********************************************************************/
#ifndef MIDDLEWARE_H
#define MIDDLEWARE_H

#include <stdint.h>
#include "../common/common_types.h"
#include "../fat_driver/fat_driver.h"

/* Error Codes */
#define MID_SUCCESS 0
#define MID_ERROR -1
#define MID_ERROR_INVALID -2
#define MID_ERROR_NOT_FOUND -3
#define MID_ERROR_ACCESS_DENIED -4
#define MID_ERROR_ALREADY_EXISTS -5
#define MID_ERROR_NOT_EMPTY -6
#define MID_ERROR_DISK_FULL -7
#define MID_ERROR_NOT_READY -8

/* Directory Node */
typedef struct dir_node {
    char name[256];              // Name
    uint8_t attributes;          // Attributes
    uint32_t size;              // Size in bytes
    uint32_t first_cluster;     // First cluster number
    uint32_t create_time;       // Creation time
    uint32_t modify_time;       // Last modification time
    struct dir_node* parent;    // Parent directory
    struct dir_node* children;  // Child nodes
    struct dir_node* next;      // Next sibling
} dir_node_t;

/* Middleware Control */
typedef struct {
    dir_node_t* root;           // Root node
    dir_node_t* current;        // Current node
    char current_path[1024];    // Current path
    uint8_t is_root_mode;       // Root/user mode
} mid_control_t;

/* Public Functions */

/* Initialization/Cleanup */
int32_t mid_init(const char* img_path, uint8_t mode);
int32_t mid_deinit(void);

/* Directory Operations */
int32_t mid_list_dir(const char* path, dir_node_t** entries, uint32_t* count);
int32_t mid_change_dir(const char* path);
int32_t mid_create_dir(const char* path);
int32_t mid_remove_dir(const char* path);

/* File Operations */
int32_t mid_read_file(const char* path, void* buffer, uint32_t size, uint32_t* bytes_read);
int32_t mid_write_file(const char* path, const void* buffer, uint32_t size, uint32_t* bytes_written);
int32_t mid_remove_file(const char* path);
int32_t mid_copy_file(const char* src_path, const char* dst_path);
int32_t mid_move_file(const char* src_path, const char* dst_path);

/* Mode Operations */
int32_t mid_set_root_mode(uint8_t enable);
int32_t mid_get_root_mode(uint8_t* enabled);

#endif /* MIDDLEWARE_H */

/*********************************************************************
 * UUID: 3b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/
