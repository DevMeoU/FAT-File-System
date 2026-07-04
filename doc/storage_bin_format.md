# storage.bin — ESP-IDF Wear-Levelled FATFS Partition Format

## Kết quả phân tích

`storage.bin` (15.138.816 bytes = 0xE70000) là một **partition FATFS của ESP-IDF được bọc trong lớp wear-levelling (WL)**, không phải image FAT thuần. Boot sector không nằm ở offset 0 mà ở offset vật lý **0x1000**.

## Bố cục vật lý

```
+--------------------------------------------------+ 0x000000
|  Data pages (4096 B/page) — FAT volume đã xoay   |
|  (logical sector 0 hiện đang ở phys 0x1000)      |
+--------------------------------------------------+ 0xE51000  addr_state1
|  wl_state copy 1: header 64 B + pos records      |
+--------------------------------------------------+ 0xE60000  addr_state2
|  wl_state copy 2 (bản mirror)                    |
+--------------------------------------------------+ 0xE6F000  addr_cfg
|  wl_config 36 B (CRC32-protected)                |
+--------------------------------------------------+ 0xE70000  (EOF)
```

Giá trị đọc được từ image:

| Trường | Giá trị |
|---|---|
| full_mem_size | 0xE70000 (15.138.816 B) |
| page_size / sector_size | 4096 |
| updaterate / wr_size | 16 / 16 |
| WL version | 2 |
| flash_size (vùng logical) | 0xE50000 (15.007.744 B) |
| max_pos | 3665; pos = 0, move_count = 0 |

## Ánh xạ logical → physical

```
phys = (flash_size - move_count*page_size + logical) % flash_size
if (phys >= pos * page_size) phys += page_size   // né dummy page
```

`pos` lưu trong state header có thể cũ; giá trị thật được khôi phục bằng cách
đếm các record 16-byte đã ghi (khác 0xFF) ngay sau header (giống
`WL_Flash::recoverPos()`). CRC32 dùng thuật toán `crc32_le(0xFFFFFFFF, ...)`
của ESP-IDF (reflected, poly 0xEDB88320).

## FAT volume bên trong (sau khi bỏ lớp WL)

| Trường | Giá trị |
|---|---|
| Loại | **FAT16** (29.049 clusters) |
| Bytes/sector | 512, 1 sector/cluster |
| Reserved / FATs / FAT size | 1 / 2 / 115 sectors |
| Root entries | 512 (root bắt đầu sector 231, data sector 263) |
| Volume label | `Espressif ` |
| Nội dung | `/lib/*.jar` (FlintOS/FlintJVM), `/sys/icons`, `/sys/fonts` |

## Hỗ trợ trong project

- `src/hal/wl_layer.[ch]` — phát hiện lớp WL (validate CRC config/state), khôi phục `pos`, dịch địa chỉ logical → physical.
- `src/hal/hal.c` — tự động phát hiện khi mount; image `.img` thuần vẫn map 1:1 như cũ.
- `src/ip_driver/ip_driver.c` — nhận thêm đuôi `.bin`, thêm API đọc/ghi theo byte-offset, fallback mở read-only.
- `src/fat_driver/fat_driver.c` — sửa kiểm tra End-of-Chain dùng `fat_driver_is_eoc()` (trước đây sai với FAT16).

Chạy: `./build/bin/application images/storage.bin`

Đã kiểm chứng: 5 file `.jar` trong image (bao gồm `java.base.jar` 1,38 MB trải ~2700 cluster) đọc ra **khớp byte-for-byte** với file gốc trong `project/files/lib/`; image FAT12 `floppy.img` vẫn hoạt động bình thường; image WL giả lập với `pos=5, move_count=3` cũng đọc chính xác.
