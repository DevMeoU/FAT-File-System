#include "application.h"
#include "../utilities/log/print_color.h"
#include "../hal/hal.h"
#include "../fat_driver/fat_driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int application_init(Application* app, Middleware* middleware) {
    if (!app || !middleware) return -1;
    
    app->middleware = middleware;
    app->running = false;
    
    return 0;
}

int application_run(Application* app, const char* img_path, FileSystemMode mode) {
    if (!app || !img_path) return -1;
    
    // Kiểm tra tham số đầu vào
    if (!img_path) {
        print_error("Image file path is required\n");
        return -1;
    }
    
    // Kiểm tra file path có tồn tại và là file img
    FILE* test_file = fopen(img_path, "rb");
    if (!test_file) {
        print_error("Image file does not exist: %s\n", img_path);
        return -1;
    }
    fclose(test_file);
    
    const char* ext = strrchr(img_path, '.');
    if (!ext || strcmp(ext, ".img") != 0) {
        print_error("File is not an image file (.img): %s\n", img_path);
        return -1;
    }
    
    // Khởi tạo HAL
    HAL hal;
    if (hal_init(&hal, img_path, SECTOR_SIZE_512) != 0) {
        print_error("Failed to initialize HAL\n");
        return -1;
    }
    
    // Cấu hình hệ thống tệp
    FileSystemConfig config;
    config.mode = mode;
    config.fat_type = FAT_TYPE_16; // Mặc định, sẽ được xác định lại trong fat_driver_mount
    config.sector_size = SECTOR_SIZE_512;
    config.cache_size = CACHE_SIZE_16;
    config.dir_name_len = DIR_NAME_LEN_8;
    
    // Khởi tạo FAT Driver
    FATDriver fat_driver;
    if (fat_driver_init(&fat_driver, &hal, config) != 0) {
        print_error("Failed to initialize FAT Driver\n");
        hal_close(&hal);
        return -1;
    }
    
    // Mount hệ thống tệp
    if (fat_driver_mount(&fat_driver) != 0) {
        print_error("Failed to mount file system\n");
        fat_driver_unmount(&fat_driver);
        hal_close(&hal);
        return -1;
    }
    
    print_success("Mount successful\n");
    
    // Khởi tạo Middleware
    Middleware middleware;
    if (middleware_init(&middleware, &fat_driver) != 0) {
        print_error("Failed to initialize Middleware\n");
        fat_driver_unmount(&fat_driver);
        hal_close(&hal);
        return -1;
    }
    
    // Chuyển chế độ dựa trên đường dẫn
    if (strcmp(img_path, "/") == 0) {
        middleware_switch_to_root_mode(&middleware);
    } else {
        middleware_switch_to_user_mode(&middleware);
    }
    
    // Gán middleware cho application
    app->middleware = &middleware;
    app->running = true;
    
    // Vòng lặp chính của ứng dụng
    char command[256];
    
    while (app->running) {
        // Hiển thị prompt
        if (middleware_is_root_mode(&middleware)) {
            print_color(COLOR_GREEN, "root");
        } else {
            print_color(COLOR_BLUE, "user");
        }
        
        printf(":%s> ", middleware_get_current_path(&middleware));
        
        // Đọc lệnh
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }
        
        // Xóa ký tự newline
        size_t len = strlen(command);
        if (len > 0 && command[len - 1] == '\n') {
            command[len - 1] = '\0';
        }
        
        // Xử lý lệnh
        application_process_command(app, command);
    }
    
    // Dọn dẹp
    fat_driver_unmount(&fat_driver);
    hal_close(&hal);
    
    return 0;
}

int application_process_command(Application* app, const char* command) {
    if (!app || !command) return -1;
    
    // Tách lệnh và tham số
    char cmd_copy[256];
    strncpy(cmd_copy, command, sizeof(cmd_copy) - 1);
    cmd_copy[sizeof(cmd_copy) - 1] = '\0';
    
    char* cmd = strtok(cmd_copy, " ");
    if (!cmd) return 0;
    
    // Xử lý các lệnh
    if (strcmp(cmd, "ls") == 0) {
        return middleware_ls(app->middleware);
    } else if (strcmp(cmd, "cd") == 0) {
        char* path = strtok(NULL, " ");
        if (!path) {
            print_error("cd: missing operand\n");
            return -1;
        }
        return middleware_cd(app->middleware, path);
    } else if (strcmp(cmd, "cat") == 0) {
        char* path = strtok(NULL, " ");
        if (!path) {
            print_error("cat: missing operand\n");
            return -1;
        }
        return middleware_cat(app->middleware, path);
    } else if (strcmp(cmd, "evidence") == 0) {
        return middleware_evidence(app->middleware);
    } else if (strcmp(cmd, "help") == 0) {
        application_show_help(app);
        return 0;
    } else if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) {
        application_stop(app);
        return 0;
    } else {
        print_error("Unknown command: %s\n", cmd);
        print_info("Type 'help' for available commands\n");
        return -1;
    }
}

void application_show_help(Application* app) {
    (void)app; // Tránh cảnh báo unused parameter
    
    printf("Available commands:\n");
    printf("  ls                  List files and directories\n");
    printf("  cd <path>           Change directory\n");
    printf("  cat <file>          Display file content\n");
    printf("  evidence            Show file system information\n");
    printf("  help                Show this help message\n");
    printf("  exit, quit          Exit the program\n");
}

void application_stop(Application* app) {
    if (!app) return;
    
    app->running = false;
    print_info("Exiting...\n");
}

// Hàm main
int main(int argc, char* argv[]) {
    // Kiểm tra tham số dòng lệnh
    if (argc < 2) {
        print_error("Usage: %s <img_file> [mode]\n", argv[0]);
        print_info("  <img_file>: Path to the image file\n");
        print_info("  [mode]: Optional, 'read-only' (default) or 'read-write'\n");
        return 1;
    }
    
    const char* img_path = argv[1];
    FileSystemMode mode = MODE_READ_ONLY; // Mặc định là read-only
    
    // Xử lý tham số mode nếu có
    if (argc >= 3) {
        if (strcmp(argv[2], "read-write") == 0) {
            mode = MODE_READ_WRITE;
        } else if (strcmp(argv[2], "read-only") == 0) {
            mode = MODE_READ_ONLY;
        } else {
            print_error("Invalid mode: %s\n", argv[2]);
            print_info("Mode must be 'read-only' or 'read-write'\n");
            return 1;
        }
    }
    
    // Khởi tạo và chạy ứng dụng
    Application app;
    Middleware middleware; // Tạm thời, sẽ được khởi tạo lại trong application_run
    
    if (application_init(&app, &middleware) != 0) {
        print_error("Failed to initialize application\n");
        return 1;
    }
    
    int result = application_run(&app, img_path, mode);
    
    return result == 0 ? 0 : 1;
}
