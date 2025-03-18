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
NC='\033[0m' # No Color

# Ví dụ in chữ có màu cầu vồng
echo -e "${RED}R${ORANGE}A${YELLOW

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
    cat <<EOF > "$CONFIG_FILE"
img_num=$img_num
auto_build=$auto_build
auto_run=$auto_run
auto_clean=$auto_clean
EOF
}

# Giá trị mặc định
img_num=${img_num:-1}
auto_build=${auto_build:-"enabled"}
auto_run=${auto_run:-"disabled"}
auto_clean=${auto_clean:-"disabled"}

#------------------
# Display functions
#------------------
function show_logo() {
    echo -e "${CYAN}"
    echo "██████╗ ███████╗███████╗███████╗ ██████╗ ██╗         ████████╗ ██████╗  ██████╗ ██╗         ██╗  ██╗██╗   ██╗██████╗ ";
    echo "██╔══██╗██╔════╝██╔════╝██╔════╝██╔═══██╗██║         ╚══██╔══╝██╔═══██╗██╔═══██╗██║         ██║  ██║██║   ██║██╔══██╗";
    echo "██║  ██║█████╗  █████╗  ███████╗██║   ██║██║            ██║   ██║   ██║██║   ██║██║         ███████║██║   ██║██████╔╝";
    echo "██║  ██║██╔══╝  ██╔══╝  ╚════██║██║   ██║██║            ██║   ██║   ██║██║   ██║██║         ██╔══██║██║   ██║██╔══██╗";
    echo "██████╔╝███████╗███████╗███████║╚██████╔╝███████╗       ██║   ╚██████╔╝╚██████╔╝███████╗    ██║  ██║╚██████╔╝██████╔╝";
    echo "╚═════╝ ╚══════╝╚══════╝╚══════╝ ╚═════╝ ╚══════╝       ╚═╝    ╚═════╝  ╚═════╝ ╚══════╝    ╚═╝  ╚═╝ ╚═════╝ ╚═════╝ ";
    echo "                                                                                                                     ";
    echo -e "${NC}"
    echo -e "${ITALIC}   Author: DEESOL${NC}"
    echo -e "${ITALIC}   Date: $(date '+%Y-%m-%d %H:%M:%S')${NC}"
}

function show_config() {
    echo -e "${YELLOW}   Minimal Shell Config${NC}"
    echo
    echo "   Auto Build:   $auto_build"
    echo "   Auto Run:     $auto_run"
    echo "   Auto Clean:   $auto_clean"
    echo "   Work Dir:     $(pwd)"
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
    ls -l ./images
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
        9) echo -e "${RED}   Exiting...${NC}"; exit 0 ;;
        *) echo -e "${RED}Invalid input, please try again!${NC}"; handle_user_choice ;;
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
        5) return 0 ;;
        *) echo -e "${RED}   Invalid choice!${NC}" ;;
    esac
}

#------------------
# Project functions
#------------------
function build() {
    echo -e "${GREEN}   Building project...${NC}"
    make all
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
    echo -e "${YELLOW}   Setting image number...${NC}"
    read -e -p "   Input image number: " input_num
    img_num=$input_num
    echo -e "${GREEN}   Image number is set to $img_num${NC}"
}

function set_auto_build() {
    echo -e "${YELLOW}   Setting auto build...${NC}"
    read -e -p "   Enable auto build? (enabled/disabled): " input_value
    auto_build=$input_value
    echo -e "${GREEN}   Auto build is set to $auto_build${NC}"
}

function set_auto_run() {
    echo -e "${YELLOW}   Setting auto run...${NC}"
    read -e -p "   Enable auto run? (enabled/disabled): " input_value
    auto_run=$input_value
    echo -e "${GREEN}   Auto run is set to $auto_run${NC}"
}

function set_auto_clean() {
    echo -e "${YELLOW}   Setting auto clean...${NC}"
    read -e -p "   Enable auto clean? (enabled/disabled): " input_value
    auto_clean=$input_value
    echo -e "${GREEN}   Auto clean is set to $auto_clean${NC}"
}

function set_config() {
    clear
    show_logo
    show_config
    show_menu_config
    handle_config_choice
    save_config
    echo -e "${GREEN}   Configuration saved!${NC}"
    sleep 1
    main
}

#------------------
# Main shell function
#------------------
function main() {
    load_config
    cd "./project" || { echo -e "${RED}Project folder not found${NC}"; exit 1; }
    show_logo
    show_config
    show_img_files
    show_menu
    handle_user_choice
}

# Khởi động shell
main