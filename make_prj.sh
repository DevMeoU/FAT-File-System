#!/bin/bash
# List of required directories as per the desired structure
required_dirs=(
    "project"
    "project/src"
    "project/src/ip_driver"
    "project/src/hal"
    "project/src/fat_driver"
    "project/src/middleware"
    "project/src/application"
    "project/src/utilities"
    "project/src/utilities/linkedlist"
)

echo "Checking and creating required directories..."
# Create required directories if they don't exist
for dir in "${required_dirs[@]}"; do
    if [ ! -d "$dir" ]; then
        echo "Creating directory: $dir"
        mkdir -p "$dir"
    else
        echo "Directory already exists: $dir"
    fi
done

# Remove extra directories in project/src (only allow: ip_driver, hal, fat_driver, middleware, application, utilities)
echo "Checking project/src for extra directories..."
if [ -d "project/src" ]; then
    cd project/src || exit 1
    for d in */ ; do
        d=${d%/}  # Remove trailing slash
        if [[ "$d" != "ip_driver" && "$d" != "hal" && "$d" != "fat_driver" && "$d" != "middleware" && "$d" != "application" && "$d" != "utilities" ]]; then
            echo "Removing extra directory in src: $d"
            rm -rf "$d"
        fi
    done
    cd - > /dev/null
fi

# Remove extra directories in project/src/utilities (only allow: linkedlist)
echo "Checking project/src/utilities for extra directories..."
if [ -d "project/src/utilities" ]; then
    cd project/src/utilities || exit 1
    for d in */ ; do
        d=${d%/}
        if [[ "$d" != "linkedlist" ]]; then
            echo "Removing extra directory in utilities: $d"
            rm -rf "$d"
        fi
    done
    cd - > /dev/null
fi

# Create sample files if they don't exist
echo "Creating sample files if they don't exist..."
touch project/src/ip_driver/ip_driver.c project/src/ip_driver/ip_driver.h
touch project/src/hal/hal.c project/src/hal/hal.h
touch project/src/fat_driver/fat_driver.c project/src/fat_driver/fat_driver.h
touch project/src/middleware/middleware.c project/src/middleware/middleware.h
touch project/src/application/application.c project/src/application/application.h
touch project/src/utilities/linkedlist/linkedlist.c project/src/utilities/linkedlist/linkedlist.h
touch project/Makefile

echo "Project structure has been updated as required."

