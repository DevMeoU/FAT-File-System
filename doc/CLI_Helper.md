# CHI TIẾT VỀ CLI HELPER (utilities/cli)

Module `cli_helper` được thiết kế để nâng cao trải nghiệm người dùng trong môi trường Command Line Interface (CLI), cung cấp các tính năng tương tự như các Shell hiện đại (bash, zsh).

## 1. Các tính năng chính

### 1.1. Tab-Completion (Tự động hoàn thành)
* **Lệnh (Commands):** Hỗ trợ gợi ý các lệnh hệ thống khi nhấn TAB (ls, cd, cat, help, ...).
* **Đường dẫn (Path/Files):** Tự động liệt kê và hoàn thành tên file/thư mục dựa trên vị trí hiện tại của người dùng trong hệ thống FAT.
* **Đường dẫn lồng nhau:** Hỗ trợ TAB cho các đường dẫn dài (ví dụ: `cd folder1/subfolder2/[TAB]`).
* **Hỗ trợ thư mục:** Tự động thêm dấu `/` sau khi hoàn thành tên thư mục.

### 1.2. Command History (Lịch sử lệnh)
* Sử dụng bộ đệm vòng (circular buffer) để lưu trữ tối đa **20 lệnh** gần nhất.
* Sử dụng phím **Mũi tên Lên (Up)** và **Xuống (Down)** để duyệt qua lịch sử.
* **Thông minh:** Không lưu các lệnh trùng lặp liên tiếp vào lịch sử.
* **Khôi phục đệm tạm:** Nếu người dùng đang gõ dở một lệnh rồi nhấn "Lên" để xem lịch sử, nội dung đang gõ dở sẽ được sao lưu và khôi phục lại khi người dùng nhấn "Xuống" hết danh sách lịch sử.

### 1.3. Cursor Navigation & Editing (Di chuyển con trỏ & Chỉnh sửa)
* **Di chuyển con trỏ:** Sử dụng phím **Mũi tên Trái (Left)** và **Phải (Right)** để di chuyển con trỏ bên trong dòng lệnh hiện tại.
* **Chỉnh sửa giữa dòng:** Cho phép chèn ký tự hoặc xóa (Backspace) tại bất kỳ vị trí nào con trỏ đang đứng. Luồng xử lý sẽ tự động dịch chuyển bộ đệm (memmove) và cập nhật hiển thị tương ứng.

## 2. Kiến trúc đa nền tảng (Cross-platform)

CLI Helper giải quyết bài toán đọc phím thời gian thực mà không cần nhấn Enter bằng cách sử dụng các API hệ thống thay vì `scanf` hoặc `fgets` tiêu chuẩn:

### 2.1. Trên Windows (`_WIN32`)
* Sử dụng `GetStdHandle(STD_INPUT_HANDLE)` kết hợp với `ReadConsoleInput`.
* Tắt chế độ `ENABLE_LINE_INPUT` và `ENABLE_ECHO_INPUT` để kiểm soát hoàn toàn việc in ký tự ra màn hình.
* Nhận diện các phím đặc biệt qua `VirtualKeyCode` (VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT).

### 2.2. Trên Linux/POSIX
* Sử dụng `termios.h` để chuyển terminal sang **Non-canonical mode** và tắt **Echo**.
* Xử lý phím mũi tên thông qua các chuỗi thoát (Escape Sequences) bắt đầu bằng ký tự ASCII 27 (`ESC`), theo sau là `[` và mã phím (A: Lên, B: Xuống, C: Phải, D: Trái).

## 3. Mã điều khiển trực quan (Visual Control)
Sử dụng các chuỗi ANSI Escape Codes để điều khiển con trỏ thật trên terminal:
* `\b`: Xóa lùi lại (Backspace).
* `\033[D`: Di chuyển con trỏ sang trái 1 vị trí.
* `\033[C`: Di chuyển con trỏ sang phải 1 vị trí.

## 4. API Chính
`int cli_get_input(Middleware* middleware, char* buffer, size_t size);`
* Đây là hàm chính thay thế cho `fgets`. Nó quản lý luồng nhập vòng lặp, xử lý từng ký tự và thực hiện các tính năng thông minh đã nêu trên.
