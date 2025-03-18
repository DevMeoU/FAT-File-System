#!/bin/bash
# DTH Shell

# Color codes
RED='\033[0;31m'
ORANGE='\033[0;33m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
INDIGO='\033[0;35m'
VIOLET='\033[1;35m'
CYAN='\033[0;36m'
# No Color
NC='\033[0m'
ITALIC='\033[3m'  # Italic text
NC='\033[0m' # No Color

#------------------
# Load and Save Config
#------------------
CONFIG_FILE="shell_config.cfg"

function load_config() {
    if [ -f "$CONFIG_FILE" ]; then
        source "$CONFIG_FILE"
    fi
}

function save_config() {
    echo -e "${GREEN}   Do you want to save the configuration?${NC}"
    read -e -p "   Save configuration? (yes[y]/no[n]): " save_config

    case "$save_config" in
        "yes"|"y")
            if [ ! -f "$CONFIG_FILE" ]; then
                echo -e "${YELLOW}   Configuration file not found. Creating new one...${NC}"
                touch "$CONFIG_FILE"
            fi

            sed -i "/^auto_build=/c\auto_build=\"$auto_build\"" "$CONFIG_FILE" || echo "auto_build=\"$auto_build\"" >> "$CONFIG_FILE"
            sed -i "/^auto_run=/c\auto_run=\"$auto_run\"" "$CONFIG_FILE" || echo "auto_run=\"$auto_run\"" >> "$CONFIG_FILE"
            sed -i "/^auto_clean=/c\auto_clean=\"$auto_clean\"" "$CONFIG_FILE" || echo "auto_clean=\"$auto_clean\"" >> "$CONFIG_FILE"
            sed -i "/^img_num=/c\img_num=\"$img_num\"" "$CONFIG_FILE" || echo "img_num=\"$img_num\"" >> "$CONFIG_FILE"

            echo -e "${GREEN}   Configuration saved successfully!${NC}"
            ;;
        *)
            echo -e "${RED}   Configuration not saved!${NC}"
            ;;
    esac
}


#------------------
# Check project folder
#------------------
# Kiểm tra thư mục project tồn tại
clear
[ -d "./project" ] || { echo -e "${RED}Project folder not found${NC}"; exit 1; }
cd "./project" || { echo -e "${RED}Project folder not found${NC}"; exit 1; }
# Đảm bảo file cấu hình tồn tại
if [ -f "$CONFIG_FILE" ]; then
    auto_build=$(awk -F '=' '/^auto_build=/ {gsub(/"/, "", $2); print $2}' "$CONFIG_FILE")
    auto_run=$(awk -F '=' '/^auto_run=/ {gsub(/"/, "", $2); print $2}' "$CONFIG_FILE")
    auto_clean=$(awk -F '=' '/^auto_clean=/ {gsub(/"/, "", $2); print $2}' "$CONFIG_FILE")
    img_num=$(awk -F '=' '/^img_num=/ {gsub(/"/, "", $2); print $2}' "$CONFIG_FILE")
else
    echo "Configuration file not found!"
    exit 1
fi

# Đặt giá trị mặc định nếu trống
auto_build=${auto_build:-"disabled"}
auto_run=${auto_run:-"disabled"}
auto_clean=${auto_clean:-"enabled"}
img_num=${img_num:-0}  # Đảm bảo giá trị số không bị lỗi

#------------------
# Display functions
#------------------
function show_logo() {
    clear
    echo -e "${RED}     ██████╗ ███████╗███████╗███████╗ ██████╗ ██╗         ████████╗ ██████╗  ██████╗ ██╗         ██╗  ██╗██╗   ██╗██████╗ ";
    echo -e "${ORANGE}     ██╔══██╗██╔════╝██╔════╝██╔════╝██╔═══██╗██║         ╚══██╔══╝██╔═══██╗██╔═══██╗██║         ██║  ██║██║   ██║██╔══██╗";
    echo -e "${YELLOW}     ██║  ██║█████╗  █████╗  ███████╗██║   ██║██║            ██║   ██║   ██║██║   ██║██║         ███████║██║   ██║██████╔╝";
    echo -e "${GREEN}     ██║  ██║██╔══╝  ██╔══╝  ╚════██║██║   ██║██║            ██║   ██║   ██║██║   ██║██║         ██╔══██║██║   ██║██╔══██╗";
    echo -e "${BLUE}     ██████╔╝███████╗███████╗███████║╚██████╔╝███████╗       ██║   ╚██████╔╝╚██████╔╝███████╗    ██║  ██║╚██████╔╝██████╔╝";
    echo -e "${INDIGO}     ╚═════╝ ╚══════╝╚══════╝╚══════╝ ╚═════╝ ╚══════╝       ╚═╝    ╚═════╝  ╚═════╝ ╚══════╝    ╚═╝  ╚═╝ ╚═════╝ ╚═════╝ ";
    echo
    echo -e "${VIOLET} ${ITALIC}   Author: DEESOL${NC}"
    echo -e "${CYAN} ${ITALIC}   Date: $(date '+%Y-%m-%d %H:%M:%S')${NC}"
    echo -e "${NC}"
}

function show_config() {
    echo -e "${YELLOW}   Minimal Shell Config${NC}"
    echo
    echo "   Work Dir:     $(pwd)"
    echo "   Auto Build:   $auto_build"
    echo "   Auto Run:     $auto_run"
    echo "   Auto Clean:   $auto_clean"
    echo "   Image number: $img_num"
}

function show_menu() {
    echo -e "${GREEN}   Menu${NC}"
    echo
    echo "   1. Build       - Build project"
    echo "   2. Run         - Run project"
    echo "   3. Clean       - Clean project"
    echo "   4. ReBuild     - ReBuild project"
    echo "   5. RunBuild    - Run and Build project"
    echo "   6. All         - Run, Build and Clean project"
    echo "   7. Reload      - Reload terminal"
    echo "   8. Config      - Set configuration"
    echo "   9. Exit        - Exit shell"
}

function show_img_files() {
    echo -e "${GREEN}   Showing image files...${NC}"
    counter=1
    for img_file in $(ls images/*.img | sort); do
        echo -e "${CYAN}   $counter. $img_file"
        ((counter++))
    done
    echo -e "${NC}"
}

function show_menu_config() {
    echo -e "${GREEN}   Configuration Menu${NC}"
    echo "   1. Set image number"
    echo "   2. Set auto build"
    echo "   3. Set auto run"
    echo "   4. Set auto clean"
    echo "   5. Back to main menu"
}

#------------------
# User input functions
#------------------
function handle_user_choice() {
    local choice
    read -e -p "Input your choice from keyboard: " choice
    echo
    case $choice in
        1) build ;;
        2) run ;;
        3) clean ;;
        4) clean; build ;;
        5) build; run ;;
        6) clean; build; run ;;
        7) reload ;;
        8) set_config ;;
        9) exit_flag="true" ;;  # Gán flag để thoát
        *) echo -e "${RED}Invalid input, please try again!${NC}";;
    esac
}

function handle_config_choice() {
    local choice
    read -e -p "   Input your choice: " choice
    case $choice in
        1) set_img_num ;;
        2) set_auto_build ;;
        3) set_auto_run ;;
        4) set_auto_clean ;;
        5) return ;;
        *) echo -e "${RED}   Invalid choice!${NC}" ;;
    esac
}

#------------------
# Project functions
#------------------
function build() {
    echo -e "${GREEN}   Building project...${NC}"
    make all &
    wait $!  # Chờ tiến trình `make` hoàn thành trước khi tiếp tục
    echo -e "${GREEN}   Build completed!${NC}"
}

function run() {
    echo -e "${GREEN}   Running project...${NC}"
    make run
}

function clean() {
    echo -e "${GREEN}   Cleaning project...${NC}"
    make clean
    echo -e "${GREEN}   Clean completed!${NC}"
}

function reload() {
    exec "$SHELL"
}

#------------------
# Configuration functions
#------------------
function set_img_num() {
    show_img_files
    echo
    echo -e "${YELLOW}   Setting image number...${NC}"
    read -e -p "   Input image number: " input_num
    img_num=$input_num
    echo -e "${GREEN}   Image number is set to $img_num${NC}"
}

function set_auto_build() {
    echo -e "${YELLOW}   Setting auto build...${NC}"
    read -e -p "   Enable auto build? (enabled[e]/disabled[d]): " input_value

    case "$input_value" in
        "enabled"|"e") auto_build="enabled" ;;
        "disabled"|"d") auto_build="disabled" ;;
        *)
            echo -e "${RED}   Invalid input!${NC}"
            set_auto_build  # Gọi lại hàm nếu nhập sai
            return
            ;;
    esac

    echo -e "${GREEN}   Auto build is set to ${auto_build}${NC}"
}

function set_auto_run() {
    echo -e "${YELLOW}   Setting auto run...${NC}"
    read -e -p "   Enable auto run? (enabled/disabled): " input_value
    case "$input_value" in
        "enabled"|"e") auto_run="enabled" ;;
        "disabled"|"d") auto_run="disabled" ;;
        *)
            echo -e "${RED}   Invalid input!${NC}"
            set_auto_build  # Gọi lại hàm nếu nhập sai
            return
            ;;
    esac
    
    echo -e "${GREEN}   Auto run is set to $auto_run${NC}"
}

function set_auto_clean() {
    echo -e "${YELLOW}   Setting auto clean...${NC}"
    read -e -p "   Enable auto clean? (enabled/disabled): " input_value
    case "$input_value" in
        "enabled"|"e") auto_clean="enabled" ;;
        "disabled"|"d") auto_clean="disabled" ;;
        *)
            echo -e "${RED}   Invalid input!${NC}"
            set_auto_build  # Gọi lại hàm nếu nhập sai
            return
            ;;
    esac

    echo -e "${GREEN}   Auto clean is set to $auto_clean${NC}"
}

function set_config() {
    show_logo
    show_config
    echo
    show_menu_config
    handle_config_choice
    save_config
    echo -e "${GREEN}   Configuration saved!${NC}"
}

function back_to_main_menu() {
    show_logo
    show_config
    show_menu
    handle_user_choice
}

#------------------
# Main shell function
#------------------
function main() {
    load_config
    show_logo
    show_config

    while true; do
        if [ "$auto_clean" == "enabled" ]; then
            clean
        fi

        if [ "$img_num" -gt 0 ]; then
            show_img_files
        fi

        if [ "$auto_build" == "enabled" ]; then
            build
        fi

        if [ "$auto_run" == "enabled" ]; then
            run
        fi

        load_config
        show_logo
        show_config
        show_menu  # Hiển thị menu sau khi chạy các tùy chọn tự động
        handle_user_choice  # Xử lý lựa chọn của người dùng

        sleep 2  # Thêm một khoảng thời gian chờ để tránh chạy quá nhanh
        # Nếu user chọn Exit thì thoát vòng lặp
        if [ "$exit_flag" == "true" ]; then
            break
        fi
    done

    echo -e "${RED}   Exiting shell...${NC}"
    exit 0  # Thoát hẳn script
}

# Khởi động shell
main