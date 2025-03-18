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

void display_prompt(Application* app) {
    // Hiển thị prompt
    if (middleware_is_root_mode(app->middleware)) {
        print_color(COLOR_BOLD COLOR_UNDERLINE COLOR_GREEN, "DEESOL");
        print_color(COLOR_BOLD, "@");
        print_color(COLOR_ITALIC COLOR_GREEN, "root: ");
    } else {
        print_color(COLOR_BOLD COLOR_UNDERLINE COLOR_GREEN, "DEESOL");
        print_color(COLOR_BOLD, "@");
        print_color(COLOR_ITALIC COLOR_BLUE, "user: ");
    }
    print_color(COLOR_MAGENTA ,"%s", middleware_get_current_path(app->middleware));
    print_color(COLOR_YELLOW,"$> ");
    fflush(stdout);
}

int application_run(Application* app) {
    // Vòng lặp chính của ứng dụng
    char command[256];
    
    while (app->running) {
        /* Hiển thị prompt */
        display_prompt(app);
        
        // Đọc lệnh
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }
        
        // Xóa ký tự newline
        size_t len = strlen(command);
        if (len > 0 && command[len - 1] == '\n') {
            command[len - 1] = '\0';
        }
        
        /* Tiền xử lý lệnh && */
        if(process_command_with_and(app, command) == 0){
            continue;
        }
        else
        {
            // Xử lý lệnh
            if(-1 == application_process_command(app, command)){
                print_error("Failed to process command\n");
            }
        }

    }
    
    middleware_denit(app->middleware);
    
    return 0;
}

// Hàm loại bỏ khoảng trắng ở đầu và cuối chuỗi
char *trim(char *str) {
    while (isspace((unsigned char)*str)) str++;  // Bỏ khoảng trắng đầu
    if (*str == 0) return str;  // Nếu chuỗi rỗng, trả về ngay

    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;  // Bỏ khoảng trắng cuối

    end[1] = '\0';  // Kết thúc chuỗi
    return str;
}

// Hàm thay thế strtok_r()
char *custom_strtok_r(char *str, const char *delim, char **saveptr) {
    if (str) {
        *saveptr = str;
    }
    if (!*saveptr) {
        return NULL;
    }

    str = *saveptr;
    char *end = strstr(str, delim);
    if (end) {
        *end = '\0';
        *saveptr = end + strlen(delim);
    } else {
        *saveptr = NULL;
    }

    return str;
}

// Hàm xử lý lệnh chứa &&
int process_command_with_and(Application* app, const char *command) {
    char command_copy[1024];  
    strcpy(command_copy, command);  // Tạo bản sao để chỉnh sửa

    if (!command) return -1;

    if(strstr(command, "&&") == NULL){
        return -1;
    }

    char *saveptr = NULL;
    char *cmd = custom_strtok_r(command_copy, "&&", &saveptr);

    while (cmd) {
        cmd = trim(cmd);
        if (*cmd != '\0') {
            // Gọi xử lý lệnh ở đây
            display_prompt(app);
            if(application_process_command(app, cmd)){
                return -1;
            }
        }
        cmd = custom_strtok_r(NULL, "&&", &saveptr);
    }
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
    } else if (strcmp(cmd, "cls") == 0 || strcmp(cmd, "clear") == 0) {
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
    printf("  cls, clear          Clear the screen\n");
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
