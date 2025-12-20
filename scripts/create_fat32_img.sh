#!/bin/bash
# Script to create a FAT32 image from a folder
# Usage: ./create_fat32_img.sh

# Configuration
SOURCE_DIR="../project/src"     # Source folder to package
IMG_NAME="../project/images/src.img" # Output image path
IMG_SIZE_MB=100                 # Size in Megabytes
MOUNT_POINT="./tmp_mount"       # Temporary mount point (for fallback method)

# Ensure we are in the script's directory
cd "$(dirname "$0")"

# Check if source directory exists
if [ ! -d "$SOURCE_DIR" ]; then
    echo "Error: Source directory $SOURCE_DIR not found."
    exit 1
fi

# Create output directory if it doesn't exist
mkdir -p "$(dirname "$IMG_NAME")"

# Remove old image if it exists
[ -f "$IMG_NAME" ] && rm -f "$IMG_NAME"

echo "Step 1: Creating empty image file ($IMG_SIZE_MB MB)..."
dd if=/dev/zero of="$IMG_NAME" bs=1M count="$IMG_SIZE_MB" status=none

echo "Step 2: Formatting image as FAT32..."
if command -v mkfs.vfat >/dev/null 2>&1; then
    mkfs.vfat -F 32 "$IMG_NAME" > /dev/null
else
    echo "Warning: mkfs.vfat not found. If on Windows, you may need mtools or a Linux environment."
fi

echo "Step 3: Copying files to image..."

# Check if mtools is available (Portable, no root required)
if command -v mcopy >/dev/null 2>&1; then
    echo "Using mtools (mcopy) to populate image..."
    # -i specifies the image, -s for recursive copy
    # Note: ::/ refers to the root of the FAT image
    mcopy -i "$IMG_NAME" -s "$SOURCE_DIR"/* ::/
    echo "SUCCESS: Image created at $IMG_NAME"
else
    echo "mtools (mcopy) not found. Attempting mount method (requires sudo/Linux)..."
    mkdir -p "$MOUNT_POINT"
    
    if sudo mount -o loop "$IMG_NAME" "$MOUNT_POINT" 2>/dev/null; then
        sudo cp -r "$SOURCE_DIR"/* "$MOUNT_POINT"/
        sync
        sudo umount "$MOUNT_POINT"
        rmdir "$MOUNT_POINT"
        echo "SUCCESS: Image created at $IMG_NAME using mount method."
    else
        echo "ERROR: Failed to mount image. Please install 'mtools' for a more portable experience."
        exit 1
    fi
fi

echo "Done."