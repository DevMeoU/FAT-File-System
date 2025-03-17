# TÀI LIỆU THIẾT KẾ HỆ THỐNG FAT FILE SYSTEM

## 1. Cấu hình chung của hệ thống

Hệ thống hỗ trợ các cấu hình biến đổi theo các thông số sau (áp dụng cho các tầng: FAT Driver, Middleware, Application):

* **Mode:**
  * Read-only
  * Read-write
* **Loại FAT:**
  * FAT12
  * FAT16
  * FAT32
* **Kích thước Sector:**
  * 512, 1024, 2048, 4096 byte
* **Kích thước Cache:**
  * 16, 32, 64, 128 sector
* **Độ dài tên thư mục:**
  * 8, 16, 32, 64 ký tự
* **Độ dài tên file:**
  * 255 ký tự
* **Số ký tự tối đa của tên file:**
  * 255 ký tự

Các tham số này được sử dụng để cấu hình toàn bộ hệ thống từ tầng FAT Driver cho đến tầng Application.

---

## 2. Kiến trúc hệ thống và cấu trúc dự án

### 2.1. Cấu trúc dự án

<pre class="!overflow-visible" data-start="873" data-end="2282"><div class="contain-inline-size rounded-md border-[0.5px] border-token-border-medium relative bg-token-sidebar-surface-primary dark:bg-gray-950"><div class="flex items-center text-token-text-secondary px-4 py-2 text-xs font-sans justify-between rounded-t-[5px] h-9 bg-token-sidebar-surface-primary dark:bg-token-main-surface-secondary select-none">scss</div><div class="sticky top-9 md:top-[5.75rem]"><div class="absolute bottom-0 right-2 flex h-9 items-center"><div class="flex items-center rounded bg-token-sidebar-surface-primary px-2 font-sans text-xs text-token-text-secondary dark:bg-token-main-surface-secondary"><span class="" data-state="closed"><button class="flex gap-1 items-center select-none px-4 py-1" aria-label="Copy"><svg width="24" height="24" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg" class="icon-xs"><path fill-rule="evenodd" clip-rule="evenodd" d="M7 5C7 3.34315 8.34315 2 10 2H19C20.6569 2 22 3.34315 22 5V14C22 15.6569 20.6569 17 19 17H17V19C17 20.6569 15.6569 22 14 22H5C3.34315 22 2 20.6569 2 19V10C2 8.34315 3.34315 7 5 7H7V5ZM9 7H14C15.6569 7 17 8.34315 17 10V15H19C19.5523 15 20 14.5523 20 14V5C20 4.44772 19.5523 4 19 4H10C9.44772 4 9 4.44772 9 5V7ZM5 9C4.44772 9 4 9.44772 4 10V19C4 19.5523 4.44772 20 5 20H14C14.5523 20 15 19.5523 15 19V10C15 9.44772 14.5523 9 14 9H5Z" fill="currentColor"></path></svg>Copy</button></span><span class="" data-state="closed"><button class="flex select-none items-center gap-1"><svg width="24" height="24" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg" class="icon-xs"><path d="M2.5 5.5C4.3 5.2 5.2 4 5.5 2.5C5.8 4 6.7 5.2 8.5 5.5C6.7 5.8 5.8 7 5.5 8.5C5.2 7 4.3 5.8 2.5 5.5Z" fill="currentColor" stroke="currentColor" stroke-linecap="round" stroke-linejoin="round"></path><path d="M5.66282 16.5231L5.18413 19.3952C5.12203 19.7678 5.09098 19.9541 5.14876 20.0888C5.19933 20.2067 5.29328 20.3007 5.41118 20.3512C5.54589 20.409 5.73218 20.378 6.10476 20.3159L8.97693 19.8372C9.72813 19.712 10.1037 19.6494 10.4542 19.521C10.7652 19.407 11.0608 19.2549 11.3343 19.068C11.6425 18.8575 11.9118 18.5882 12.4503 18.0497L20 10.5C21.3807 9.11929 21.3807 6.88071 20 5.5C18.6193 4.11929 16.3807 4.11929 15 5.5L7.45026 13.0497C6.91175 13.5882 6.6425 13.8575 6.43197 14.1657C6.24513 14.4392 6.09299 14.7348 5.97903 15.0458C5.85062 15.3963 5.78802 15.7719 5.66282 16.5231Z" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"></path><path d="M14.5 7L18.5 11" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"></path></svg>Edit</button></span></div></div></div><div class="overflow-y-auto p-4" dir="ltr"><code class="!whitespace-pre"><span>project
  └── src
       ├── ip_driver
       │     ├── ip_driver.h     // Định nghĩa API thao tác I/O cấp thấp (ví dụ: đọc/ghi file ảnh floppy.img)
       │     └── ip_driver.c     // Triển khai các hàm I/O trực tiếp với file ảnh
       ├── hal
       │     ├── hal.h           // API trừu tượng hóa đọc/ghi sector (không phụ thuộc vào phương thức I/O cụ thể)
       │     └── hal.c           // Triển khai các hàm gọi tới IP Driver và cung cấp giao diện cho tầng trên
       ├── fat_driver
       │     ├── fat_driver.h    // Định nghĩa API xử lý hệ thống tệp FAT (FAT12, FAT16, FAT32)
       │     └── fat_driver.c    // Triển khai logic xử lý FAT: đọc boot sector, tính toán FAT table, root directory, file data, …
       ├── middleware
       │     ├── middleware.h    // API xử lý logic nghiệp vụ (phân tích đường dẫn, điều hướng, ls, cd,…)
       │     └── middleware.c    // Xử lý dữ liệu từ FAT Driver, chuyển đổi dữ liệu cho tầng Application
       ├── application
       │     ├── application.h   // Định nghĩa API cho ứng dụng, giao diện người dùng (shell, lệnh,…)
       │     └── application.c   // Hàm main() và giao diện người dùng, xử lý lệnh và chuyển yêu cầu xuống tầng Middleware
       └── utilities
             └── linkedlist
                   ├── linkedlist.h   // API danh sách liên kết hỗ trợ FAT Driver
                   └── linkedlist.c   // Triển khai danh sách liên kết
</span></code></div></div></pre>

### 2.2. Luồng truy cập giữa các tầng

* **Tầng Application (Ứng dụng):**
  * Giao diện shell trong application.c nhận lệnh từ người dùng (ví dụ: envidence, ls, cd, cat, …).
  * Ứng dụng kiểm tra tham số (đường dẫn file img, mode: read-only/read-write) và chuyển yêu cầu xuống tầng Middleware.
* **Tầng Middleware:**
  * Xử lý logic nghiệp vụ: phân tích lệnh, chuyển đổi đường dẫn, kiểm tra quyền, …
  * Chuyển các yêu cầu của người dùng thành các API cụ thể của FAT Driver (ví dụ: ls, cd, …).
  * Hỗ trợ chuyển đổi giữa chế độ root và chế độ user.
* **Tầng FAT Driver:**
  * Nhận yêu cầu từ Middleware để mount hệ thống tệp FAT.
  * Kiểm tra các thông số truyền vào (mode, loại FAT, kích thước sector, cache, độ dài tên thư mục, …).
  * Gọi HAL để đọc boot sector và lấy thông số hệ thống.
  * Tính toán và xây dựng các bảng:
    * FAT table (bảng phân bổ cluster)
    * Root directory table (thư mục gốc)
    * File directory table (thư mục con)
    * File data table (dữ liệu file)
  * Load toàn bộ cây thư mục vào bộ nhớ qua danh sách liên kết (với các nút chứa thông tin file, dữ liệu file nếu cần).
  * Cung cấp API điều hướng (ls, cd, …) và truy xuất file cho tầng Middleware.
* **Tầng HAL (Hardware Abstraction Layer):**
  * Trừu tượng hóa các thao tác đọc/ghi sector.
  * Gọi tới IP Driver để truy xuất trực tiếp với file ảnh (ví dụ: floppy.img).
* **Tầng IP Driver:**
  * Cung cấp các hàm I/O cấp thấp, thao tác trực tiếp với file ảnh (mở file, đọc/ghi sector, …).

---

## 3. Tầng FAT Driver

### 3.1. Yêu cầu chức năng

* **Kiểm tra tham số đầu vào:**
  * File path phải tồn tại và là file ảnh (img file).
  * Mode phải là "read-only" hoặc "read-write".
  * Kiểm tra các tham số cấu hình: loại FAT, kích thước sector, cache, độ dài tên thư mục, ….
* **Mount file system:**
  * Gọi HAL để đọc boot sector.
  * Lấy thông số từ boot sector: byte per sector, sectors per cluster, số sector dự phòng, số FAT, ….
  * Tính toán các thông số cần thiết:
    * Số sector của thư mục gốc.
    * Số sector dành cho dữ liệu.
  * Xây dựng các bảng FAT:
    * FAT table
    * Root directory table
    * File directory table
    * File data table
* **Mount mode:**
  * Nếu mode là read-only: chỉ cho phép đọc.
  * Nếu mode là read-write: cho phép đọc và ghi.
* **Thông báo kết quả mount:**
  * Nếu mount không thành công (do file không tồn tại, định dạng sai, tham số không hợp lệ) → hiển thị thông báo lỗi và thoát.
  * Nếu mount thành công → hiển thị “Mount thành công” và chuyển sang chế độ shell.
* **Chế độ khi mount thành công:**
  * Nếu file path truyền vào là "/" → chuyển sang chế độ root.
  * Nếu file path khác "/" → chuyển sang chế độ user.
  * Phân tích path truyền vào để tìm đường dẫn thực sự của file hoặc thư mục (sử dụng các hàm phân tích, điều hướng trong cây thư mục).

### 3.2. Yêu cầu cài đặt

* Mount file system phải được xây dựng để:
  * Kiểm tra các tham số cấu hình.
  * Gọi HAL để đọc boot sector.
  * Tính toán các bảng FAT, root directory, file directory, file data.
  * Load toàn bộ cây thư mục và dữ liệu file (nếu cần) vào bộ nhớ qua danh sách liên kết.

---

## 4. Tầng Middleware

### 4.1. Yêu cầu chức năng

* **Xử lý logic nghiệp vụ:**
  * Nhận các lệnh từ tầng Application (ls, cd, …).
  * Dựa trên mode và cấu hình hệ thống để chuyển yêu cầu đến FAT Driver.
* **Điều hướng thư mục:**
  * Khi lệnh `ls` → hiển thị toàn bộ file và thư mục trong thư mục hiện tại.
  * Khi lệnh `cd` → kiểm tra đường dẫn có tồn tại và là thư mục; nếu không, hiển thị lỗi và thoát.
  * Hỗ trợ các lệnh:
    * `cd /`: chuyển về thư mục gốc.
    * `cd .` hoặc `cd ./`: giữ nguyên thư mục hiện tại.
    * `cd ..` hoặc `cd ../`: chuyển về thư mục cha.
    * `cd ../abc`, `cd ../abc/def`, …: chuyển sang thư mục tương ứng trong thư mục cha.
* **Thông báo trạng thái:**
  * Khi chuyển sang chế độ root → hiển thị “Chế độ root”.
  * Khi chuyển sang chế độ user → hiển thị “Chế độ user”.

---

## 5. Tầng Application

### 5.1. Yêu cầu chức năng

* **Giao diện người dùng:**
  * Nhận các tham số từ dòng lệnh:
    * File path (img file)
    * Mode: "read-only" hoặc "read-write"
  * Kiểm tra tham số đầu vào:
    * Nếu file path không tồn tại hoặc không phải là file img → hiển thị lỗi và thoát.
    * Nếu mode không hợp lệ → hiển thị lỗi và thoát.
* **Khởi tạo hệ thống:**
  * Gọi FAT Driver (qua Middleware) để mount file system.
  * Nếu mount thành công → hiển thị “Mount thành công” và chuyển sang chế độ shell.
* **Chế độ khi mount:**
  * Nếu file path truyền vào là "/" → chuyển sang chế độ root.
  * Nếu không → chuyển sang chế độ user.

---

## 6. Quy trình chạy hệ thống

1. **Khởi động:**
   * Application nhận tham số (file path, mode).
   * Kiểm tra tồn tại file và định dạng file (img).
2. **Mount file system (FAT Driver):**
   * Kiểm tra các tham số cấu hình (mode, loại FAT, sector size, cache size, dir name length, file name length/max, …).
   * Gọi HAL để đọc boot sector và lấy thông số hệ thống.
   * Tính toán các thông số cần thiết: số sector cho FAT, root directory, file data, …
   * Xây dựng các bảng: FAT table, root directory, file directory, file data.
   * Load toàn bộ cây thư mục (và dữ liệu file nếu cần) vào bộ nhớ qua danh sách liên kết.
   * Nếu mount thành công → hiển thị thông báo “Mount thành công” và chuyển sang chế độ shell.
3. **Shell Mode (Middleware & Application):**
   * Người dùng nhập các lệnh (ls, cd, cat, …).
   * Lệnh `ls`:
     * Nếu thư mục hiện tại là "/" → hiển thị toàn bộ file và thư mục trong root.
     * Nếu khác "/" → hiển thị nội dung thư mục tương ứng.
   * Lệnh `cd`:
     * Kiểm tra đường dẫn có tồn tại và là thư mục hay không; nếu không → hiển thị lỗi và thoát.
     * Hỗ trợ các lệnh: `cd /`, `cd .`, `cd ..`, `cd ../abc`, `cd ../abc/def`, …
   * Lệnh đọc file (ví dụ: `cat /abc/def/file.txt`):
     * FAT Driver tìm kiếm file trong cây thư mục đã load.
     * Nếu file được tìm thấy và dữ liệu đã được load → hiển thị nội dung file.
4. **Thông báo và chuyển chế độ:**
   * Nếu mount thành công và file path là "/" → chuyển sang chế độ root.
   * Nếu mount thành công và file path không phải là "/" → chuyển sang chế độ user.

---

## 7. Lưu ý bổ sung

* **Kiểm tra tham số đầu vào:**

  Mỗi tầng (FAT Driver, Middleware, Application) cần xác nhận các tham số (file path, mode, cấu hình hệ thống) được truyền vào để đảm bảo tính nhất quán.
* **Phân tích đường dẫn:**

  Sau khi mount thành công, hệ thống cần phân tích đường dẫn truyền vào để xác định chế độ (root/user) và xác định đường dẫn thực sự của file hoặc thư mục.
* **Thông báo lỗi:**

  Nếu file path không tồn tại, không phải file img, hoặc nếu mount thất bại, chương trình hiển thị thông báo lỗi và thoát.
* **Chuyển đổi chế độ:**

  Sau khi mount, nếu file path là "/" thì chuyển sang chế độ root; nếu không thì chuyển sang chế độ user. Các lệnh điều hướng (ls, cd, …) sẽ kiểm tra tính hợp lệ của đường dẫn tương ứng.

---

## 8. Luồng truy cập giữa các tầng

* **Tầng Application:**
  * Nhận lệnh từ người dùng qua giao diện shell.
  * Kiểm tra tham số (file path, mode) và chuyển yêu cầu xuống tầng Middleware.
* **Tầng Middleware:**
  * Xử lý logic nghiệp vụ: phân tích lệnh, chuyển đổi đường dẫn, kiểm tra quyền.
  * Gọi các API của FAT Driver (evidence , cd, cat, …).
  * Khởi tạo cây thư mục lưu evn_path (ví dụ /home/) và lưu thông tin đến node đầu tiên của cây thư mục
    Ví dụ root gồm những thông tin như
    Name: "root"
    File class: Directory
    File size: 7126
  * Sub
    Name: app
    File class: Directory
    File size: 512
    Start cluster: 6
    Start sector: 37
    Date created: N/A
  * Date modify: 5/26/2008 3:24:30 PM
  * In ra thông tin đọc được từ FAT Driver
* **Tầng FAT Driver:**
  * Nhận yêu cầu từ Middleware để đọc hệ thống tệp FAT.
  * Gọi HAL để đọc boot sector, bảng FAT, root directory, file directory, file data.
  * Load cây thư mục và dữ liệu file (nếu cần) vào bộ nhớ qua danh sách liên kết.
  * Cung cấp API cho Middleware để điều hướng, liệt kê và đọc file.
* **Tầng HAL:**
  * Trừu tượng hóa thao tác đọc/ghi sector.
  * Gọi các hàm từ IP Driver để thực hiện I/O trên file img.
* **Tầng IP Driver:**
  * Cung cấp các hàm I/O cấp thấp thao tác trực tiếp với file ảnh (ví dụ: mở file, đọc/ghi sector).
