/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 *********************************************************************/
#include "middleware.h"
#include "application.h"

#define MAX_CMD_LEN 256
char current_path[256] = "/";  // Thư mục hiện tại, bắt đầu từ gốc

/**
 * @brief Initializes the application.
 * 
 * This function initializes the entire system by calling the respective
 * initialization functions for each layer. It ensures that the system is
 * in a valid state before the main loop starts.
 * 
 * @return 0 if initialization is successful, -1 otherwise.
 */
int app_init() {
    /* File floppy image path */
    const char *img_path = "E:\\Workspace\\project\\clone\\FATFileSystem\\images\\floppy.img";

    if (middleware_init(img_path, current_path) != 0) {
        fprintf(stderr, "middleware_init() failed!\n");
        return -1;
    }
    return 0;
}

/**
 * @brief Executes the main command loop for the application.
 * 
 * This function displays a shell prompt, reads user input commands, 
 * and handles them. The loop continues until the user inputs the 
 * "exit" command, at which point the function breaks out of the loop.
 */
void app_run() {
    char command[MAX_CMD_LEN];
    app_print_title(current_path);
    while (fgets(command, MAX_CMD_LEN, stdin)) {
        command[strcspn(command, "\n")] = 0;  // Xóa ký tự xuống dòng
        if (strcmp(command, "exit") == 0) break;
        app_handle_command(command);
        app_print_title(current_path);
    }
}

void app_handle_command(char *command) {
    char *token = strtok(command, " ");
    if (token == NULL) return;

    if (strcmp(token, "ls") == 0) {
        char *path = strtok(NULL, " ");
        if (path == NULL) path = current_path;  // Mặc định dùng thư mục hiện tại
        middleware_list_directory(path);
    } else if (strcmp(token, "cd") == 0) {
        char *path = strtok(NULL, " ");
        if (path != NULL) {
            middleware_change_directory(path, current_path);
        } else {
            printf("Please specify a path!\n");
        }
    } else if (strcmp(token, "cat") == 0) {
        char *filename = strtok(NULL, " ");
        if (filename != NULL) {
            middleware_read_file(filename);
        } else {
            printf("Please specify a file name!\n");
        }
    } else if (strcmp(token, "cls") == 0) {
        app_clear_screen();
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
    } else if (strcmp(token, "help") == 0) {
        app_print_help();
    } else {
        printf("Invalid command: %s\n", token);
    }
}

int main(void) {
    printf("Hello from application!\n");
    fflush(stdout);
    app_init();  // Khởi tạo hệ thống
    app_run();   // Chạy shell
    return 0;
}

void app_print_title(char *current_path) {
    print_colored("FATShell ", ANSI_COLOR_GREEN, ANSI_BG_BLACK);
    print_colored("> ", ANSI_COLOR_MAGENTA, ANSI_BG_BLACK);
    print_colored(current_path, ANSI_COLOR_YELLOW, ANSI_BG_BLACK);
    printf(" ");
    fflush(stdout);
}

void app_print_help() {
    printf("Available commands:\n");
    printf("ls [directory]\n");
    printf("cd [directory]\n");
    printf("cat [file]\n");
    printf("cls\n");
    printf("exit\n");
}

void app_clear_screen() {
    system("cls");
}