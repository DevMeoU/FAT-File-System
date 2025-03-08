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
#include "linkedlist.h"
#include "print_color.h"

/*********************************************************************
 * Private Variables
 *********************************************************************/

/* Danh sách đường dẫn (cây thư mục) */
static linkedlist_t *path_list = NULL;

/* Buffer dữ liệu */
static uint8_t data_buffer[MID_BUFFER_SIZE];

/*********************************************************************
 * Public Function Implementations
 *********************************************************************/

int32_t mid_init(void)
{
    /* Khởi tạo danh sách liên kết */
    path_list = llist_init();
    if (path_list == NULL) {
        log_error("Failed to initialize linked list");
        return MID_NO_MEMORY;
    }

    /* Khởi tạo FAT driver */
    fat_boot_sector_t boot_sector;
    // TODO: Load boot sector
    if (fat_init(&boot_sector) != FAT_SUCCESS) {
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
    if (fat_open(path, FAT_MODE_READ, &file) != FAT_SUCCESS) {
        return MID_NOT_FOUND;
    }

    /* Đọc dữ liệu */
    int32_t status = fat_read(&file, buffer, size, bytes_read);
    fat_close(&file);

    return (status == FAT_SUCCESS) ? MID_SUCCESS : MID_ERROR;
}

int32_t mid_write_file(const char *path, const void *buffer, uint32_t size, uint32_t *bytes_written)
{
    if (path == NULL || buffer == NULL || bytes_written == NULL) {
        return MID_INVALID;
    }

    /* Mở file */
    fat_file_t file;
    if (fat_open(path, FAT_MODE_WRITE | FAT_MODE_CREATE, &file) != FAT_SUCCESS) {
        return MID_ERROR;
    }

    /* Ghi dữ liệu */
    int32_t status = fat_write(&file, buffer, size, bytes_written);
    fat_close(&file);

    return (status == FAT_SUCCESS) ? MID_SUCCESS : MID_ERROR;
}

int32_t mid_send_data(const void *data, uint32_t size, uint32_t timeout)
{
    if (data == NULL) {
        return MID_INVALID;
    }

    /* Tạo gói tin */
    ip_packet_t packet;
    memcpy(packet.data, data, size);
    packet.size = size;

    /* Gửi dữ liệu */
    return (ip_driver_send(&packet) == 0) ? MID_SUCCESS : MID_ERROR;
}

int32_t mid_receive_data(void *buffer, uint32_t size, uint32_t *bytes_received, uint32_t timeout)
{
    if (buffer == NULL || bytes_received == NULL) {
        return MID_INVALID;
    }

    /* Nhận gói tin */
    ip_packet_t packet;
    if (ip_driver_receive(&packet, timeout) != 0) {
        return MID_TIMEOUT;
    }

    /* Copy dữ liệu */
    *bytes_received = (packet.size <= size) ? packet.size : size;
    memcpy(buffer, packet.data, *bytes_received);

    return MID_SUCCESS;
}

/*********************************************************************
 * UUID: 8b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/
