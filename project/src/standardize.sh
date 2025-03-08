#!/bin/bash

# Danh sách các module cần chuẩn hóa
MODULES=(
    "hal"
    "ip_driver"
    "fat_driver"
    "utilities/log"
    "utilities/status"
    "utilities/linkedlist"
    "middleware"
    "application"
)

# Template files
HEADER_TEMPLATE="template/header_template.h"
PRIVATE_HEADER_TEMPLATE="template/private_header_template.h"
SOURCE_TEMPLATE="template/source_template.c"
PRIVATE_SOURCE_TEMPLATE="template/private_source_template.c"
README_TEMPLATE="template/README_template.md"

# Hàm chuẩn hóa file header
standardize_header() {
    local file=$1
    local module_name=$2
    
    # Copy template
    cp $HEADER_TEMPLATE $file
    
    # Thay thế tên module
    sed -i "s/module_name/$module_name/g" $file
    sed -i "s/MODULE_NAME/${module_name^^}/g" $file
}

# Hàm chuẩn hóa file private header
standardize_private_header() {
    local file=$1
    local module_name=$2
    
    # Copy template
    cp $PRIVATE_HEADER_TEMPLATE $file
    
    # Thay thế tên module
    sed -i "s/module_name/$module_name/g" $file
    sed -i "s/MODULE_NAME/${module_name^^}/g" $file
}

# Hàm chuẩn hóa file source
standardize_source() {
    local file=$1
    local module_name=$2
    
    # Copy template
    cp $SOURCE_TEMPLATE $file
    
    # Thay thế tên module
    sed -i "s/module_name/$module_name/g" $file
}

# Hàm chuẩn hóa file private source
standardize_private_source() {
    local file=$1
    local module_name=$2
    
    # Copy template
    cp $PRIVATE_SOURCE_TEMPLATE $file
    
    # Thay thế tên module
    sed -i "s/module_name/$module_name/g" $file
}

# Hàm chuẩn hóa file README
standardize_readme() {
    local file=$1
    local module_name=$2
    
    # Copy template
    cp $README_TEMPLATE $file
    
    # Thay thế tên module
    sed -i "s/module_name/$module_name/g" $file
}

# Chuẩn hóa từng module
for module in "${MODULES[@]}"; do
    echo "Chuẩn hóa module $module..."
    
    # Lấy tên ngắn của module
    module_name=$(basename $module)
    
    # Chuẩn hóa các file
    if [ -f "$module/${module_name}.h" ]; then
        standardize_header "$module/${module_name}.h" "$module_name"
    fi
    
    if [ -f "$module/${module_name}_private.h" ]; then
        standardize_private_header "$module/${module_name}_private.h" "$module_name"
    fi
    
    if [ -f "$module/${module_name}.c" ]; then
        standardize_source "$module/${module_name}.c" "$module_name"
    fi
    
    if [ -f "$module/${module_name}_private.c" ]; then
        standardize_private_source "$module/${module_name}_private.c" "$module_name"
    fi
    
    if [ -f "$module/README.md" ]; then
        standardize_readme "$module/README.md" "$module_name"
    fi
done

echo "Hoàn thành chuẩn hóa!" 