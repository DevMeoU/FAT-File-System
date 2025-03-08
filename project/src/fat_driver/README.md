# FAT Driver Module

## Mô tả
Module FAT Driver cung cấp các hàm để đọc/ghi dữ liệu trên hệ thống tập tin FAT (File Allocation Table). Module hỗ trợ các phiên bản FAT12, FAT16 và FAT32.

## Cấu trúc thư mục
```
fat_driver/
├── fat_driver.h         - File header công khai
├── fat_driver_private.h - File header riêng
├── fat_driver.c         - File nguồn chính
├── fat_driver_private.c - File nguồn riêng
└── README.md           - File tài liệu
```

## API công khai
### Khởi tạo
```c
int32_t fat_init(const fat_boot_sector_t *boot_sector);
```
Khởi tạo FAT driver với thông tin từ boot sector.

### Thao tác tập tin
```c
int32_t fat_open(const char *path, uint8_t mode, fat_file_t *file);
int32_t fat_close(fat_file_t *file);
int32_t fat_read(fat_file_t *file, void *buffer, uint32_t size, uint32_t *bytes_read);
int32_t fat_write(fat_file_t *file, const void *buffer, uint32_t size, uint32_t *bytes_written);
int32_t fat_seek(fat_file_t *file, int32_t offset, int32_t origin);
```
Các hàm thao tác với tập tin: mở, đóng, đọc, ghi và di chuyển con trỏ.

### Thao tác thư mục
```c
int32_t fat_mkdir(const char *path);
int32_t fat_rmdir(const char *path);
```
Các hàm thao tác với thư mục: tạo và xóa thư mục.

### Thao tác chung
```c
int32_t fat_stat(const char *path, fat_file_info_t *info);
int32_t fat_unlink(const char *path);
```
Các hàm thao tác chung: lấy thông tin và xóa tập tin.

## Kiểu dữ liệu
### Cấu trúc boot sector
```c
typedef struct {
    uint8_t  jump_boot[3];        /* Mã nhảy khởi động */
    uint8_t  oem_name[8];         /* Tên OEM */
    uint16_t bytes_per_sector;    /* Số byte mỗi sector */
    uint8_t  sectors_per_cluster; /* Số sector mỗi cluster */
    ...
} fat_boot_sector_t;
```

### Cấu trúc entry thư mục
```c
typedef struct {
    uint8_t  name[11];           /* Tên tập tin */
    uint8_t  attributes;         /* Thuộc tính */
    uint8_t  reserved;           /* Dự trữ */
    ...
} fat_dir_entry_t;
```

### Cấu trúc thông tin tập tin
```c
typedef struct {
    uint8_t  name[256];         /* Tên đầy đủ */
    uint8_t  attributes;        /* Thuộc tính */
    uint32_t size;             /* Kích thước */
    ...
} fat_file_info_t;
```

### Handle tập tin
```c
typedef struct {
    fat_file_info_t info;      /* Thông tin tập tin */
    uint32_t position;         /* Vị trí đọc/ghi */
    uint32_t cluster;          /* Cluster hiện tại */
    ...
} fat_file_t;
```

## Ví dụ sử dụng
### Đọc tập tin
```c
fat_file_t file;
uint8_t buffer[512];
uint32_t bytes_read;

/* Mở tập tin */
if (fat_open("/test.txt", FAT_MODE_READ, &file) == FAT_SUCCESS) {
    /* Đọc dữ liệu */
    if (fat_read(&file, buffer, sizeof(buffer), &bytes_read) == FAT_SUCCESS) {
        /* Xử lý dữ liệu */
    }
    
    /* Đóng tập tin */
    fat_close(&file);
}
```

### Ghi tập tin
```c
fat_file_t file;
const uint8_t data[] = "Hello World!";
uint32_t bytes_written;

/* Mở tập tin */
if (fat_open("/test.txt", FAT_MODE_WRITE | FAT_MODE_CREATE, &file) == FAT_SUCCESS) {
    /* Ghi dữ liệu */
    if (fat_write(&file, data, sizeof(data), &bytes_written) == FAT_SUCCESS) {
        /* Xử lý kết quả */
    }
    
    /* Đóng tập tin */
    fat_close(&file);
}
```

## Lưu ý
- Module yêu cầu khởi tạo trước khi sử dụng các hàm khác
- Các đường dẫn phải bắt đầu bằng dấu "/"
- Tên tập tin và thư mục phải tuân theo quy tắc 8.3
- Cần đóng tập tin sau khi sử dụng xong

## Tác giả
- Ducson9112k

## UUID
6b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9 