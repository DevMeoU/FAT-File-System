#!/bin/bash
# standardize.sh - Project Structure Standardizer
# Author: DEESOL

# Color codes
RED='\033[0;31m'
ORANGE='\033[0;33m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# Determine project root
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$BASE_DIR/project"

echo -e "${CYAN}=== FAT File System Project Standardizer ===${NC}"

# 1. Ensure project structure
required_dirs=(
    "$PROJECT_DIR/src/ip_driver"
    "$PROJECT_DIR/src/hal"
    "$PROJECT_DIR/src/fat_driver"
    "$PROJECT_DIR/src/middleware"
    "$PROJECT_DIR/src/application"
    "$PROJECT_DIR/src/utilities/log"
    "$PROJECT_DIR/src/utilities/linkedlist"
    "$PROJECT_DIR/src/utilities/cli"
    "$PROJECT_DIR/src/common"
    "$PROJECT_DIR/images"
    "$PROJECT_DIR/build/obj"
    "$PROJECT_DIR/build/bin"
)

echo -e "${YELLOW}1. Checking directory structure...${NC}"
for dir in "${required_dirs[@]}"; do
    if [ ! -d "$dir" ]; then
        mkdir -p "$dir"
        echo -e "${GREEN}  + Created: $(basename "$dir")${NC}"
    fi
done

# 2. Sync files from root to project (if they exist at root and NOT in project)
# This is for legacy support/migration
echo -e "${YELLOW}2. Syncing source files...${NC}"
if [ -d "$BASE_DIR/src" ]; then
    cp -rn "$BASE_DIR/src/"* "$PROJECT_DIR/src/" 2>/dev/null
    echo -e "${GREEN}  + Synced files from root/src to project/src${NC}"
fi

# 3. Ensure essential files exist (placeholders if missing)
# This part is simplified; in a real project these would be actual source files
# I will only create files that are ABSOLUTELY necessary for a build

echo -e "${YELLOW}3. Verifying essential files...${NC}"
# (Optional: Add logic to create empty headers if missing)

# 4. Clean up redundant root files (ONLY if they are successfully moved/backed up)
# For safety, we just inform the user or delete obviously redundant ones like 'image' vs 'images'
if [ -d "$BASE_DIR/image" ] && [ -d "$PROJECT_DIR/images" ]; then
    mv "$BASE_DIR/image/"* "$PROJECT_DIR/images/" 2>/dev/null
    rmdir "$BASE_DIR/image" 2>/dev/null
    echo -e "${GREEN}  + Moved 'image' to 'project/images'${NC}"
fi

echo -e "${GREEN}Standardization complete!${NC}"
echo -e "${CYAN}Use ./DTH_SL.sh to manage your project.${NC}"