/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Module Middleware cung cấp các hàm trung gian để khởi tạo hệ thống,
 *   liệt kê thư mục, thay đổi thư mục và đọc file dựa trên FAT Driver.
 *********************************************************************/

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdio.h>
#include <string.h>
#include "middleware.h"
#include "fat_driver.h"
#include "print_color.h"

/* Default storage file path */
#define DEFAULT_STORAGE_FILE "floppy.img"

/*********************************************************************
 * Public Function Implementations
 *********************************************************************/

int32_t mid_init(void)
{
    /* Use default storage file */
    return mid_init_with_file(DEFAULT_STORAGE_FILE);
}

int32_t mid_init_with_file(const char *file_path)
{
    if (!file_path) {
        return MID_INVALID;
    }

    /* Khởi tạo IP driver */
    ip_config_t ip_config = {
        .file_path = file_path,
        .base_addr = 0,
        .irq_num = 0,
        .use_dma = false
    };
    if (ip_driver_init(&ip_config) != IP_SUCCESS) {
        log_error("Failed to initialize IP driver");
        return MID_ERROR;
    }

    /* Khởi tạo FAT driver */
    fat_config_t fat_config = {0};
    // TODO: Load configuration
    if (fat_init(&fat_config) != STATUS_SUCCESS) {
        log_error("Failed to initialize FAT driver");
        return MID_ERROR;
    }

    return MID_SUCCESS;
}

int32_t mid_process_sync(const mid_cmd_t *cmd, mid_resp_t *resp)
{
    if (cmd == NULL || resp == NULL) {
        return MID_INVALID;
    }

    /* Xử lý lệnh */
    switch (cmd->type) {
        case MID_CMD_READ:
            return mid_read_file((const char *)cmd->data, resp->data, cmd->size, &resp->size);
        
        case MID_CMD_WRITE:
            return mid_write_file((const char *)cmd->data, cmd->data + strlen((const char *)cmd->data) + 1,
                                cmd->size - strlen((const char *)cmd->data) - 1, &resp->size);
        
        case MID_CMD_SEND:
            return mid_send_data(cmd->data, cmd->size, cmd->timeout);
        
        case MID_CMD_RECEIVE:
            return mid_receive_data(resp->data, cmd->size, &resp->size, cmd->timeout);
        
        default:
            return MID_INVALID;
    }
}

int32_t mid_process_async(const mid_cmd_t *cmd, mid_callback_t callback)
{
    if (cmd == NULL || callback == NULL) {
        return MID_INVALID;
    }

    // TODO: Implement async processing
    return MID_ERROR;
}

int32_t mid_read_file(const char *path, void *buffer, uint32_t size, uint32_t *bytes_read)
{
    if (path == NULL || buffer == NULL || bytes_read == NULL) {
        return MID_INVALID;
    }

    /* Mở file */
    fat_file_t file;
    if (fat_open(path, FAT_MODE_READ, &file) != STATUS_SUCCESS) {
        return MID_NOT_FOUND;
    }

    /* Đọc dữ liệu */
    int32_t status = fat_read(&file, buffer, size, bytes_read);
    fat_close(&file);

    return (status == STATUS_SUCCESS) ? MID_SUCCESS : MID_ERROR;
}

int32_t mid_write_file(const char *path, const void *buffer, uint32_t size, uint32_t *bytes_written)
{
    if (path == NULL || buffer == NULL || bytes_written == NULL) {
        return MID_INVALID;
    }

    /* Mở file */
    fat_file_t file;
    if (fat_open(path, FAT_MODE_WRITE | FAT_MODE_CREATE, &file) != STATUS_SUCCESS) {
        return MID_ERROR;
    }

    /* Ghi dữ liệu */
    int32_t status = fat_write(&file, buffer, size, bytes_written);
    fat_close(&file);

    return (status == STATUS_SUCCESS) ? MID_SUCCESS : MID_ERROR;
}

int32_t mid_send_data(const void *data, uint32_t size, uint32_t timeout)
{
    if (data == NULL || size == 0) {
        return MID_INVALID;
    }

    (void)timeout; // Unused parameter
    // TODO: Implement data sending
    return MID_ERROR;
}

int32_t mid_receive_data(void *buffer, uint32_t size, uint32_t *bytes_received, uint32_t timeout)
{
    if (buffer == NULL || size == 0 || bytes_received == NULL) {
        return MID_INVALID;
    }

    (void)timeout; // Unused parameter
    // TODO: Implement data receiving
    return MID_ERROR;
}

/*********************************************************************
 * UUID: 8b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/
