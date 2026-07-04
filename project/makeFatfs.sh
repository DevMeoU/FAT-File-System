#!/bin/bash
set -euo pipefail

SCRIPT="fatfs/wl_fatfsgen.py"
INPUT_DIR="files"
OUTPUT="storage.bin"
DEST="images"
PART_SIZE="0xE70000"     # ~14.44 MB
SECTOR_SIZE="512"

# Kiểm tra script Python tồn tại
if [ ! -f "$SCRIPT" ]; then
    echo "Lỗi: không tìm thấy script '$SCRIPT'"
    exit 1
fi

# Kiểm tra thư mục nguồn
if [ ! -d "$INPUT_DIR" ]; then
    echo "Lỗi: không tìm thấy thư mục nguồn '$INPUT_DIR'"
    exit 1
fi

# Chạy wl_fatfsgen.py
python "$SCRIPT" \
    --output_file "$OUTPUT" \
    --partition_size "$PART_SIZE" \
    --sector_size "$SECTOR_SIZE" \
    --long_name_support \
    "$INPUT_DIR"

echo "Đã tạo '$OUTPUT' thành công."

cp "$OUTPUT" "$DEST/"