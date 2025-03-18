#include "application.h"

int application_init(Application* app, Middleware* middleware) {
    if (!app || !middleware) return -1;
    
    app->middleware = middleware;
    app->running = false;
    if(-1 != middleware_init(middleware)){
        app->running = true;
        return 0;
    }
    
    return -1;
}

int application_run(Application* app) {
    // Vòng lặp chính của ứng dụng
    char command[256];
    
    while (app->running) {
        // Hiển thị prompt
        if (middleware_is_root_mode(app->middleware)) {
            print_color(COLOR_GREEN, "DEESOL");
            print_color(COLOR_BOLD, "@");
            print_color(COLOR_GREEN, "root: ");
        } else {
            print_color(COLOR_GREEN, "DEESOL");
            print_color(COLOR_BOLD, "@");
            print_color(COLOR_BLUE, "user: ");
        }
        print_color(COLOR_MAGENTA ,"%s", middleware_get_current_path(app->middleware));
        print_color(COLOR_YELLOW,"$> ");
        fflush(stdout);
        
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
    
    middleware_denit(app->middleware);
    
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
    } else if (strcmp(cmd, "cls") == 0) {
        system("clear");
        return 0;
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
    printf("  cls                 Clear the screen\n");
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
        print_warning("Usage: %s <img_file> [mode]\n", argv[0]);
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
    Middleware middleware = {
        .img_path = img_path,
        .mode = mode,
        .fat_driver = NULL,
        .current_directory = NULL,
        .current_path = "/", // Thư mục hiện tại là root
        .is_root_mode = false
    };

    if (application_init(&app, &middleware) != 0) {
        print_error("Failed to initialize application\n");
        return 1;
    }
    
    int result = application_run(&app);
    
    return result == 0 ? 0 : 1;
}
