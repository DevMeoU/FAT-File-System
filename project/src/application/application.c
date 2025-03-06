/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 *********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "application.h"
#include "middleware.h"

#define MAX_CMD_LEN 256
char current_path[256] = "~/";  // Thư mục hiện tại, bắt đầu từ gốc

/**
 * @brief Executes the main command loop for the application.
 * 
 * This function displays a shell prompt, reads user input commands, 
 * and handles them. The loop continues until the user inputs the 
 * "exit" command, at which point the function breaks out of the loop.
 */
void app_run() {
    char command[MAX_CMD_LEN];
    printf("FATShell> ");
    while (fgets(command, MAX_CMD_LEN, stdin)) {
        command[strcspn(command, "\n")] = 0;  // Xóa ký tự xuống dòng
        if (strcmp(command, "exit") == 0) break;
        app_handle_command(command);
        printf("FATShell> ");
    }
}

void app_handle_command(char *command) {
    char *token = strtok(command, " ");
    if (token == NULL) return;

    if (strcmp(token, "ls") == 0) {
        char *path = strtok(NULL, " ");
        if (path == NULL) path = current_path;  // Mặc định dùng thư mục hiện tại
        printf("Listing directory %s:\n", path);
        // middleware_list_directory(path);
    } else if (strcmp(token, "cd") == 0) {
        char *path = strtok(NULL, " ");
        if (path != NULL) {
            printf("Changing directory to %s\n", path);
            // middleware_change_directory(path, current_path);
        } else {
            printf("Please specify a path!\n");
        }
    } else if (strcmp(token, "cat") == 0) {
        char *filename = strtok(NULL, " ");
        if (filename != NULL) {
            printf("Content of file %s:\n", filename);
            // middleware_read_file(filename);
        } else {
            printf("Please specify a file name!\n");
        }
    } else if (strcmp(token, "cls") == 0) {
        printf("Clearing screen...\n");
        // middleware_clear_screen();
    } else if (strcmp(token, "mkdir") == 0) {
        char *dirname = strtok(NULL, " ");
        if (dirname != NULL) {
            printf("Creating directory %s\n", dirname);
            // middleware_create_directory(dirname);
        } else {
            printf("Please specify a directory name!\n");
        }
    } else if (strcmp(token, "rm") == 0) {
        char *filename = strtok(NULL, " ");
        if (filename != NULL) {
            printf("Deleting file %s\n", filename);
            // middleware_delete_file(filename);
        } else {
            printf("Please specify a file name!\n");
        }
    } else if (strcmp(token, "exit()") == 0) {
        printf("Goodbye!\n");
        exit(0);
    } else {
        printf("Invalid command: %s\n", token);
    }
}

int main(void) {
    printf("Hello from application!\n");
    // app_init();  // Khởi tạo hệ thống
    app_run();   // Chạy shell
    // app_exit();  // Dọn dẹp và thoát
    return 0;
}