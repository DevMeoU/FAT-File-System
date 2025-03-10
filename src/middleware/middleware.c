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
#include "../utilities/linkedlist/linkedlist.h"
#include "middleware.h"
#include "../fat_driver/fat_driver.h"
#include "../utilities/log/print_color.h"

/*********************************************************************
 * Private Variables
 *********************************************************************/
/* List of Manager Node */
linkedlist_t manager_node_list;

/* Information Of Manager Node */
mid_node_info_t manager_node_info;

/*********************************************************************
 * Public Function Implementations
 *********************************************************************/

int32_t mid_init(const char* img_path)
{
    /* Khởi tạo FAT driver */
    if (fat_driver_init(img_path) != STATUS_SUCCESS) {
        print_error("Failed to initialize FAT driver\n");
        return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}

int32_t mid_init_with_file(const char *img_file_path, const char *env_path)
{
    /* Khởi tạo FAT driver */
    if (fat_driver_init(img_file_path) != STATUS_SUCCESS) {
        print_error("Failed to initialize FAT driver\n");
        return STATUS_ERROR;
    }

    /* Lưu đường dẫn environment */
    manager_node_info.env_path = strdup(env_path);
    if (manager_node_info.env_path == NULL) {
        print_error("Failed to allocate memory for environment path\n");
        return STATUS_NO_MEMORY;
    }

    return STATUS_SUCCESS;
}

int32_t mid_deinit(void)
{
    /* Giải phóng FAT driver */
    if (fat_driver_deinit() != STATUS_SUCCESS) {
        print_error("Failed to deinitialize FAT driver\n");
        return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}

int32_t mid_process_sync(const mid_cmd_t *cmd, mid_resp_t *resp)
{
    if (cmd == NULL || resp == NULL) {
        return STATUS_INVALID_PARAMETER;
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
            return STATUS_INVALID_PARAMETER;
    }
}

int32_t mid_process_async(const mid_cmd_t *cmd, mid_callback_t callback)
{
    if (cmd == NULL || callback == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    // TODO: Implement async processing
    return STATUS_ERROR;
}

int32_t mid_read_file(const char *path, void *buffer, uint32_t size, uint32_t *bytes_read)
{
    if (path == NULL || buffer == NULL || bytes_read == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    /* Mở file */
    fat_entry_t file;
    if (fat_driver_open(path, FAT_MODE_READ, &file) != STATUS_SUCCESS) {
        return STATUS_NOT_FOUND;
    }

    /* Đọc dữ liệu */
    int32_t status = fat_driver_read(&file, buffer, size, bytes_read);
    fat_driver_close(&file);

    return status;
}

int32_t mid_write_file(const char *path, const void *buffer, uint32_t size, uint32_t *bytes_written)
{
    if (path == NULL || buffer == NULL || bytes_written == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    /* Mở file */
    fat_entry_t file;
    if (fat_driver_open(path, FAT_MODE_WRITE | FAT_MODE_CREATE, &file) != STATUS_SUCCESS) {
        return STATUS_ERROR;
    }

    /* Ghi dữ liệu */
    int32_t status = fat_driver_write(&file, buffer, size, bytes_written);
    fat_driver_close(&file);

    return status;
}

int32_t mid_send_data(const void *data, uint32_t size, uint32_t timeout)
{
    if (data == NULL || size == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    (void)timeout; // Unused parameter
    // TODO: Implement data sending
    return STATUS_ERROR;
}

int32_t mid_receive_data(void *buffer, uint32_t size, uint32_t *bytes_received, uint32_t timeout)
{
    if (buffer == NULL || size == 0 || bytes_received == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    (void)timeout; // Unused parameter
    // TODO: Implement data receiving
    return STATUS_ERROR;
}

int32_t mid_init_mw(Middleware* mw, FATDriver* driver)
{
    if (!mw || !driver) {
        return STATUS_INVALID_PARAMETER;
    }

    mw->fat_driver = driver;
    strcpy(mw->current_path, "/");
    mw->current_dir_entries = NULL;
    mw->current_dir_count = 0;

    // Đọc thư mục gốc
    return mid_change_directory(mw, "/");
}

int32_t mid_list_directory(Middleware* mw)
{
    if (!mw) {
        return STATUS_INVALID_PARAMETER;
    }

    // In danh sách file/thư mục
    for (uint32_t i = 0; i < mw->current_dir_count; i++) {
        fat_entry_t* entry = &mw->current_dir_entries[i];
        printf("%s\t%s\t%u bytes\n",
               entry->attributes & FAT_ATTR_DIRECTORY ? "DIR" : "FILE",
               entry->name,
               entry->size);
    }

    return STATUS_SUCCESS;
}

int32_t mid_change_directory(Middleware* mw, const char* path)
{
    if (!mw || !path) {
        return STATUS_INVALID_PARAMETER;
    }

    char full_path[1024];
    int len = snprintf(full_path, sizeof(full_path), "%s/%s", mw->current_path, path);
    if (len >= sizeof(full_path)) {
        return STATUS_ERROR;
    }

    // Xử lý các trường hợp đặc biệt
    if (strcmp(path, "/") == 0) {
        // Chuyển về thư mục gốc
        strcpy(mw->current_path, "/");
        return fat_driver_read_root_dir(mw->fat_driver);
    }
    
    if (strcmp(path, ".") == 0) {
        // Giữ nguyên thư mục hiện tại
        return STATUS_SUCCESS;
    }

    if (strcmp(path, "..") == 0) {
        // Lên thư mục cha
        char* last_slash = strrchr(mw->current_path, '/');
        if (last_slash != mw->current_path) {
            *last_slash = '\0';
        }
        return mid_change_directory(mw, mw->current_path);
    }

    // Tìm và chuyển đến thư mục
    if (fat_driver_find_path(mw->fat_driver, full_path) == STATUS_SUCCESS) {
        strcpy(mw->current_path, full_path);
        return STATUS_SUCCESS;
    }

    return STATUS_ERROR;
}

int32_t mid_read_file_content(Middleware* mw, const char* filename, void* buffer, uint32_t* size)
{
    if (!mw || !filename || !buffer || !size) {
        return STATUS_INVALID_PARAMETER;
    }

    char full_path[1024];
    int len = snprintf(full_path, sizeof(full_path), "%s/%s", mw->current_path, filename);
    if (len >= sizeof(full_path)) {
        return STATUS_ERROR;
    }

    return fat_driver_read_file(mw->fat_driver, full_path, buffer, size);
}

void mid_cleanup(Middleware* mw)
{
    if (!mw) {
        return;
    }

    // Giải phóng bộ nhớ
    if (mw->current_dir_entries != NULL) {
        free(mw->current_dir_entries);
        mw->current_dir_entries = NULL;
    }

    // Reset các trường
    mw->fat_driver = NULL;
    mw->current_path[0] = '\0';
    mw->current_dir_count = 0;
}

/*********************************************************************
 * UUID: 8b9c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/
