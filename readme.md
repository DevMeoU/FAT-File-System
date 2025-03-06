**1. Tổng Quan Kiến Trúc Sau Sửa Đổi**

**Mục tiêu**

* **Xây dựng ứng dụng C chạy trên Windows để đọc và xử lý file **floppy.img** (hệ thống tệp FAT12).**
* **Mô phỏng giao tiếp phần cứng thông qua thao tác trên file.**
* **Cho phép người dùng tương tác với hệ thống tệp FAT qua terminal bằng các lệnh giống Linux (**ls**, **cd**, **cat**, v.v.).**
* **Quản lý đường dẫn dưới dạng chuỗi string thay vì danh sách liên kết.**

**Luồng Dữ Liệu**

* **Dữ liệu vẫn truyền qua các tầng: **Application → Middleware → FAT Driver → HAL → IP Driver** và ngược lại.**
* **Tuy nhiên, Application giờ đây sẽ là một shell CLI, nhận lệnh từ người dùng và gửi yêu cầu với đường dẫn string xuống các tầng dưới.**

**Cấu Trúc Thư Mục**

**Cấu trúc dự án giữ nguyên như mô tả ban đầu:**

```text
project
  └── src
       ├── ip_driver       // I/O cấp thấp trên file floppy.img
       ├── hal            // Trừu tượng hóa đọc/ghi sector
       ├── fat_driver     // Xử lý logic hệ thống tệp FAT
       ├── middleware     // Xử lý logic nghiệp vụ và ánh xạ lệnh
       ├── application    // Giao diện CLI và điều phối
       └── utilities      // Công cụ hỗ trợ (có thể bỏ linkedlist nếu không dùng)
```

---

**2. Chi Tiết Các Thay Đổi Theo Từng Tầng**

**2.1. Application Layer**

**Chức Năng Chính**

* **Chuyển từ giao diện menu số sang giao diện CLI với prompt (ví dụ: **FATShell>**).**
* **Nhận lệnh từ người dùng (như **ls**, **cd**, **cat**) và gửi yêu cầu với đường dẫn string đến Middleware.**
* **Quản lý thư mục hiện tại (**current_path**) dưới dạng chuỗi string, bắt đầu từ **"~/"** (thư mục gốc).**

**Triển Khai**

**Dưới đây là mã ví dụ trong **application.c**:**

**c**

```c
#include"application.h"
#include"middleware.h"
#include<stdio.h>
#include<string.h>

#defineMAX_CMD_LEN256
char current_path[256]="~/";// Thư mục hiện tại, bắt đầu từ gốc

voidapp_run(){
char command[MAX_CMD_LEN];
printf("FATShell> ");
while(fgets(command, MAX_CMD_LEN,stdin)){
        command[strcspn(command,"\n")]=0;// Xóa ký tự xuống dòng
if(strcmp(command,"exit")==0)break;
app_handle_command(command);
printf("FATShell> ");
}
}

voidapp_handle_command(char*command){
char*token =strtok(command," ");
if(token ==NULL)return;

if(strcmp(token,"ls")==0){
char*path =strtok(NULL," ");
if(path ==NULL) path = current_path;// Mặc định dùng thư mục hiện tại
middleware_list_directory(path);
}elseif(strcmp(token,"cd")==0){
char*path =strtok(NULL," ");
if(path !=NULL){
middleware_change_directory(path, current_path);
}else{
printf("Vui lòng chỉ định đường dẫn!\n");
}
}elseif(strcmp(token,"cat")==0){
char*filename =strtok(NULL," ");
if(filename !=NULL){
middleware_read_file(filename);
}else{
printf("Vui lòng chỉ định tên file!\n");
}
}else{
printf("Lệnh không hợp lệ: %s\n", token);
}
}

intmain(void){
app_init();// Khởi tạo hệ thống
app_run();// Chạy shell
app_exit();// Dọn dẹp và thoát
return0;
}
```

**Giải Thích**

* **app_run()**: Hiển thị prompt và nhận lệnh từ người dùng trong vòng lặp.
* **app_handle_command()**: Phân tích lệnh, gọi các hàm Middleware tương ứng với đường dẫn thích hợp.
* **current_path**: Biến toàn cục theo dõi thư mục hiện tại, được cập nhật khi dùng lệnh **cd**.

---

**2.2. Middleware Layer**

**Chức Năng Chính**

* **Nhận lệnh và đường dẫn từ Application, ánh xạ thành các thao tác trên FAT Driver.**
* **Quản lý logic nghiệp vụ và xử lý đường dẫn string thay vì danh sách liên kết.**

**Triển Khai**

**Mã ví dụ trong **middleware.c**:**

**c**

```c
#include"middleware.h"
#include"fat_driver.h"
#include<stdio.h>
#include<string.h>

voidmiddleware_list_directory(constchar*path){
printf("Danh sách file/thư mục tại %s:\n", path);
// Gọi FAT Driver để liệt kê thư mục tại path
fat_driver_list_directory(path);// Truyền string path trực tiếp
// In kết quả (giả sử FAT Driver xử lý việc hiển thị)
}

voidmiddleware_change_directory(constchar*path,char*current_path){
// Kiểm tra và cập nhật đường dẫn
if(fat_driver_directory_exists(path)){
strcpy(current_path, path);
printf("Đã chuyển đến thư mục: %s\n", path);
}else{
printf("Thư mục không tồn tại: %s\n", path);
}
}

voidmiddleware_read_file(constchar*filename){
// Gọi FAT Driver để đọc file
printf("Nội dung của file %s:\n", filename);
fat_driver_read_file(filename);// Truyền string filename trực tiếp
// In kết quả (giả sử FAT Driver xử lý việc hiển thị)
}
```

**Giải Thích**

* **Các hàm nhận đường dẫn string từ Application và gọi FAT Driver với tham số tương ứng.**
* **middleware_change_directory()**: Cập nhật **current_path** nếu đường dẫn hợp lệ.

---

**2.3. FAT Driver Layer**

**Chức Năng Chính**

* **Xử lý logic hệ thống tệp FAT12, nhận đường dẫn string từ Middleware.**
* **Thay vì trả về danh sách liên kết, các hàm sẽ trực tiếp xử lý và hiển thị (hoặc trả dữ liệu qua buffer).**

**Triển Khai**

**Mã ví dụ trong **fat_driver.c**:**

**c**

```c
#include"fat_driver.h"
#include"hal.h"
#include<stdio.h>
#include<string.h>

intfat_driver_list_directory(constchar*path){
// Logic phân tích path để tìm thư mục
// Đọc sector chứa thư mục qua HAL, phân tích các entry
// Ví dụ: In trực tiếp danh sách file/thư mục
unsignedchar buffer[512];
hal_read_sector(1, buffer);// Giả sử sector 1 chứa thư mục gốc
// Xử lý và in các entry (tên file, kích thước, v.v.)
printf("file1.txt  512 bytes\n");// Ví dụ minh họa
printf("dir1      <DIR>\n");
return0;// Thành công
}

intfat_driver_read_file(constchar*filename){
// Logic tìm file theo filename trong thư mục hiện tại
// Đọc chuỗi cluster qua HAL và in nội dung
unsignedchar buffer[512];
hal_read_sector(2, buffer);// Giả sử sector 2 chứa dữ liệu file
printf("%s\n", buffer);// In nội dung file
return0;// Thành công
}

intfat_driver_directory_exists(constchar*path){
// Logic kiểm tra path có phải thư mục hợp lệ
// Đọc sector chứa thư mục, kiểm tra entry
return1;// Giả sử tồn tại (cần triển khai thực tế)
}
```

**Giải Thích**

* **Các hàm được điều chỉnh để nhận đường dẫn string và gọi HAL với sector tương ứng.**
* **Logic phân tích đường dẫn (tương đối hoặc tuyệt đối) cần được triển khai dựa trên cấu trúc FAT12.**

---

**2.4. HAL và IP Driver**

* **HAL**: Không thay đổi nhiều, vẫn chuyển đổi yêu cầu từ sector sang offset và gọi IP Driver.
* **IP Driver**: Không thay đổi, vẫn xử lý I/O cấp thấp trên file **floppy.img** với offset và size.

**Ví Dụ HAL**

**c**

```c
voidhal_read_sector(unsignedint sector,unsignedchar*buffer){
unsignedint offset = sector *512;// Giả sử sector size = 512 bytes
ip_driver_read(offset, buffer,512);
}
```

**Ví Dụ IP Driver**

**c**

```c
intip_driver_read(unsignedint offset,unsignedchar*buffer,size_t size){
fseek(file, offset,SEEK_SET);
size_t bytes_read =fread(buffer,1, size, file);
return(bytes_read == size)?0:-1;
}
```

---

**2.5. Quản Lý Đường Dẫn**

* **Cách thức**: Sử dụng chuỗi string (**current_path**) trong Application để theo dõi thư mục hiện tại.
* **Xử lý**:
  * **Lệnh **cd dir1**: Cập nhật **current_path** thành **"~/dir1"** nếu **dir1** tồn tại.**
  * **Lệnh **ls**: Liệt kê nội dung tại **current_path** nếu không chỉ định đường dẫn.**
* **Lưu ý**: Cần hàm helper để chuẩn hóa đường dẫn (ví dụ: xử lý **..** để quay lại thư mục cha).

---

**3. Luồng Hoạt Động Minh Họa**

1. **Người dùng chạy chương trình → Hiển thị **FATShell>**.**
2. **Gõ **ls** → Middleware gọi FAT Driver liệt kê thư mục tại **"~/"** → In danh sách file/thư mục.**
3. **Gõ **cd dir1** → Middleware kiểm tra và cập nhật **current_path** thành **"~/dir1"**.**
4. **Gõ **cat file.txt** → Middleware gọi FAT Driver đọc nội dung **file.txt** tại **"~/dir1"** → In nội dung.**
5. **Gõ **exit** → Thoát chương trình.**

---

**4. Tóm Tắt Thay Đổi**

* **Application**: Chuyển sang CLI với prompt, hỗ trợ lệnh **ls**, **cd**, **cat**, quản lý **current_path**.
* **Middleware**: Nhận và xử lý đường dẫn string, gọi FAT Driver tương ứng.
* **FAT Driver**: Điều chỉnh để nhận string, xử lý logic FAT12 với đường dẫn.
* **HAL và IP Driver**: Không thay đổi lớn, vẫn hỗ trợ đọc/ghi sector.
* **Utilities**: Bỏ danh sách liên kết nếu không cần thiết.

---

**5. Lưu Ý Khi Triển Khai**

* **Chuẩn hóa đường dẫn**: Cần xử lý đường dẫn tương đối (**dir1**) và tuyệt đối (**~/dir1**).
* **Hiệu suất**: Có thể thêm cache trong Middleware để tối ưu.
* **Kiểm thử**: Test từng lệnh (**ls**, **cd**, **cat**) với file **floppy.img** thực tế.
