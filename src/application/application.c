/*********************************************************************
 * Module Application - Triển khai giao diện người dùng
 *********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "application.h"
#include "../fat_driver/fat_driver.h"

#define CMD_MAX_LEN 256
#define MAX_ARGS 16
#define APP_PATH_MAX 1024

/* Cấu trúc lệnh */
typedef struct {
    const char* name;
    const char* help;
    int (*handler)(Application* app, int argc, char* argv[]);
} Command;

/* Các hàm xử lý lệnh */
static int cmd_ls(Application* app, int argc, char* argv[]) {
    const char* path = (argc > 1) ? argv[1] : app->current_path;
    fat_dir_t dir;
    fat_dir_entry_t entry;
    
    if (fat_opendir(path, &dir) != FAT_SUCCESS) {
        printf("Lỗi: Không thể đọc thư mục %s\n", path);
        return APP_ERROR;
    }

    printf("\nNội dung thư mục %s:\n", path);
    printf("%-32s %8s %s\n", "Tên", "Kích thước", "Thuộc tính");
    printf("----------------------------------------\n");
    
    while (fat_readdir(&dir, &entry) == FAT_SUCCESS) {
        char attr[5] = "----";
        if (entry.attr & FAT_ATTR_DIRECTORY) attr[0] = 'd';
        if (entry.attr & FAT_ATTR_READ_ONLY) attr[1] = 'r';
        if (entry.attr & FAT_ATTR_HIDDEN) attr[2] = 'h';
        if (entry.attr & FAT_ATTR_SYSTEM) attr[3] = 's';
        
        char name[13];
        memcpy(name, entry.name, 11);
        name[11] = '\0';
        
        printf("%-32s %8u %s\n", 
               name,
               entry.file_size,
               attr);
    }
    
    fat_closedir(&dir);
    return APP_SUCCESS;
}

static int cmd_cd(Application* app, int argc, char* argv[]) {
    if (argc != 2) {
        printf("Sử dụng: cd <thư_mục>\n");
        return APP_ERROR;
    }

    char new_path[APP_PATH_MAX];
    
    // Xử lý các trường hợp đặc biệt
    if (strcmp(argv[1], "/") == 0) {
        // Chuyển về root
        strcpy(app->current_path, "/");
        app->is_root_mode = 1;
        printf("Đã chuyển sang chế độ root\n");
        return APP_SUCCESS;
    } 
    else if (strcmp(argv[1], ".") == 0 || strcmp(argv[1], "./") == 0) {
        // Giữ nguyên thư mục hiện tại
        return APP_SUCCESS;
    }
    else if (strcmp(argv[1], "..") == 0 || strcmp(argv[1], "../") == 0) {
        // Chuyển về thư mục cha
        char* last_slash = strrchr(app->current_path, '/');
        if (last_slash == app->current_path) {
            strcpy(app->current_path, "/");
            app->is_root_mode = 1;
            printf("Đã chuyển sang chế độ root\n");
        } else {
            *last_slash = '\0';
        }
        return APP_SUCCESS;
    }
    else if (strncmp(argv[1], "../", 3) == 0) {
        // Xử lý đường dẫn tương đối (../abc/def)
        char* path = strdup(argv[1] + 3);
        char* token = strtok(path, "/");
        
        // Chuyển về thư mục cha trước
        char* last_slash = strrchr(app->current_path, '/');
        if (last_slash == app->current_path) {
            strcpy(app->current_path, "/");
            app->is_root_mode = 1;
        } else {
            *last_slash = '\0';
        }
        
        // Duyệt qua từng phần của đường dẫn
        while (token) {
            strcat(app->current_path, "/");
            strcat(app->current_path, token);
            token = strtok(NULL, "/");
        }
        
        free(path);
        return APP_SUCCESS;
    }
    
    // Xử lý đường dẫn tuyệt đối
    if (argv[1][0] == '/') {
        strncpy(new_path, argv[1], APP_PATH_MAX-1);
    } else {
        // Xử lý đường dẫn tương đối
        snprintf(new_path, APP_PATH_MAX-1, "%s/%s", 
                app->current_path[1] ? app->current_path : "", argv[1]);
    }
    
    // Kiểm tra thư mục tồn tại
    fat_dir_t dir;
    if (fat_opendir(new_path, &dir) != FAT_SUCCESS) {
        printf("Lỗi: Thư mục không tồn tại\n");
        return APP_ERROR;
    }
    fat_closedir(&dir);
    
    strcpy(app->current_path, new_path);
    app->is_root_mode = (strcmp(app->current_path, "/") == 0);
    
    if (app->is_root_mode) {
        printf("Đã chuyển sang chế độ root\n");
    } else {
        printf("Đã chuyển sang chế độ user\n");
    }
    
    return APP_SUCCESS;
}

static int cmd_help(Application* app, int argc, char* argv[]) {
    (void)app;
    (void)argc;
    (void)argv;
    
    printf("\nCác lệnh được hỗ trợ:\n\n");
    printf("ls [path]      - Liệt kê nội dung thư mục\n");
    printf("cd <path>      - Thay đổi thư mục hiện tại\n");
    printf("help           - Hiển thị trợ giúp\n");
    printf("exit           - Thoát chương trình\n\n");
    
    printf("Các đường dẫn hỗ trợ:\n");
    printf("  /          - Thư mục gốc\n");
    printf("  ./         - Thư mục hiện tại\n");
    printf("  ../        - Thư mục cha\n");
    printf("  ../abc     - Thư mục abc trong thư mục cha\n");
    printf("  /abc/def   - Đường dẫn tuyệt đối\n\n");
    return APP_SUCCESS;
}

static int cmd_exit(Application* app, int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    app->running = 0;
    return APP_SUCCESS;
}

/* Bảng lệnh */
static Command commands[] = {
    {"ls", "Liệt kê nội dung thư mục", cmd_ls},
    {"cd", "Thay đổi thư mục hiện tại", cmd_cd},
    {"help", "Hiển thị trợ giúp", cmd_help},
    {"exit", "Thoát chương trình", cmd_exit},
    {NULL, NULL, NULL}
};

/* Xử lý lệnh người dùng */
static int handle_command(Application* app, const char* cmd_str) {
    char cmd_buf[CMD_MAX_LEN];
    char* argv[MAX_ARGS];
    int argc = 0;
    
    strncpy(cmd_buf, cmd_str, CMD_MAX_LEN - 1);
    cmd_buf[CMD_MAX_LEN - 1] = '\0';
    
    char* token = strtok(cmd_buf, " ");
    while (token && argc < MAX_ARGS) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    
    if (argc == 0) return APP_SUCCESS;
    
    for (Command* cmd = commands; cmd->name != NULL; cmd++) {
        if (strcmp(argv[0], cmd->name) == 0) {
            return cmd->handler(app, argc, argv);
        }
    }
    
    printf("Lỗi: Lệnh không hợp lệ '%s'\n", argv[0]);
    return APP_ERROR;
}

/* Kiểm tra file img */
static int check_img_file(const char* path) {
    FILE* fp = fopen(path, "rb");
    if (!fp) return 0;
    
    // Đọc boot sector để kiểm tra định dạng FAT
    uint8_t boot[512];
    if (fread(boot, 1, 512, fp) != 512) {
        fclose(fp);
        return 0;
    }
    
    // Kiểm tra chữ ký boot sector
    if (boot[510] != 0x55 || boot[511] != 0xAA) {
        fclose(fp);
        return 0;
    }
    
    // Kiểm tra định dạng FAT từ OEM Name
    const char* fat_sig = (const char*)&boot[3];
    if (strncmp(fat_sig, "MSDOS5.0", 8) != 0 &&
        strncmp(fat_sig, "FAT32   ", 8) != 0 &&
        strncmp(fat_sig, "FAT16   ", 8) != 0) {
        fclose(fp);
        return 0;
    }
    
    fclose(fp);
    return 1;
}

/* Khởi tạo cấu hình mặc định */
static void init_default_config(AppConfig* config) {
    // Sử dụng giá trị mặc định
    config->mode = MODE_READ_ONLY;
    config->sector_size = 512;
    config->cache_size = 16;
    config->dir_name_len = 8;
}

/* Khởi tạo ứng dụng */
int application_init(Application* app, const char* img_path, const char* mode) {
    if (!app || !img_path || !mode) return APP_ERROR;
    
    // Kiểm tra file img
    if (!check_img_file(img_path)) {
        printf("Lỗi: File %s không tồn tại hoặc không phải file img\n", img_path);
        return APP_ERROR;
    }
    
    // Kiểm tra mode
    if (strcmp(mode, MODE_READ_ONLY) != 0 && strcmp(mode, MODE_READ_WRITE) != 0) {
        printf("Lỗi: Mode không hợp lệ (phải là 'ro' hoặc 'rw')\n");
        return APP_ERROR;
    }
    
    // Khởi tạo cấu hình mặc định
    init_default_config(&app->config);
    app->config.mode = mode;
    
    // Khởi tạo FAT
    if (fat_init(img_path) != FAT_SUCCESS) {
        printf("Lỗi: Không thể đọc file img\n");
        return APP_ERROR;
    }
    
    app->running = 1;
    app->img_path = strdup(img_path);
    app->current_path = strdup("/");
    app->is_root_mode = 1;
    
    printf("Đọc file img thành công!\n");
    printf("Chế độ: %s\n", app->is_root_mode ? "root" : "user");
    
    return APP_SUCCESS;
}

/* Chạy shell */
void application_run_shell(Application* app) {
    char cmd_buf[CMD_MAX_LEN];
    
    printf("\nFAT File System Shell v1.0\n");
    printf("Nhập 'help' để xem danh sách lệnh\n\n");
    
    while (app->running) {
        printf("%s> ", app->current_path);
        if (fgets(cmd_buf, CMD_MAX_LEN, stdin)) {
            cmd_buf[strcspn(cmd_buf, "\n")] = 0;
            handle_command(app, cmd_buf);
        }
    }
}

/* Dọn dẹp */
void application_cleanup(Application* app) {
    if (!app) return;
    if (app->img_path) free(app->img_path);
    if (app->current_path) free(app->current_path);
    fat_deinit();
}

/* Hàm main */
int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Sử dụng: %s <đường_dẫn_img> <mode>\n", argv[0]);
        printf("Mode: ro (read-only) hoặc rw (read-write)\n");
        return 1;
    }
    
    Application app;
    if (application_init(&app, argv[1], argv[2]) != APP_SUCCESS) {
        return 1;
    }
    
    application_run_shell(&app);
    application_cleanup(&app);
    
    return 0;
}

/*********************************************************************
 * UUID: 4f8d2e1c-9b4a-4e85-8c6d-f7b2e3a1d5c9
 *********************************************************************/