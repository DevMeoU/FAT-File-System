#!/bin/bash
# DTH_Shell.sh - Unified Shell for FAT Project
# Author: DEESOL

# Load common utilities
SOURCE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SOURCE_DIR/common_utils.sh"

# Set language (passed as $1 or default to EN)
LANG_MODE=${1:-"EN"}
CONFIG_FILE="$BASE_DIR/shell_config.cfg"
PID_LOG="$PROJECT_DIR/.processes.pid"

# -----------------------
# LOCALIZATION STRINGS
# -----------------------
if [[ "$LANG_MODE" == "VI" ]]; then
    STR_AUTHOR="Tác giả"
    STR_DATE="Ngày"
    STR_MENU="Menu"
    STR_BUILD="Build"
    STR_RUN="Chạy"
    STR_CLEAN="Clean"
    STR_REBUILD="ReBuild"
    STR_RUNBUILD="Build và Chạy"
    STR_ALL="All (Clean, Build, Run)"
    STR_RELOAD="Tải lại Shell"
    STR_CONFIG="Cấu hình"
    STR_EXIT="Thoát"
    STR_IMG_LIST="Danh sách file ảnh..."
    STR_IMG_SELECT="Chọn file ảnh..."
    STR_INPUT="Nhập lựa chọn của bạn: "
    STR_INVALID="Lựa chọn không hợp lệ!"
    STR_BUILDING="Đang build project..."
    STR_RUNNING="Khởi chạy chương trình..."
    STR_STOPPING="Đang dừng các tiến trình..."
    STR_SUCCESS="Thành công!"
    STR_FAILED="Thất bại!"
    STR_SAVE_CFG="Bạn có muốn lưu cấu hình không?"
    STR_PRESS_KEY="Nhấn phím bất kỳ để tiếp tục..."
else
    STR_AUTHOR="Author"
    STR_DATE="Date"
    STR_MENU="Menu"
    STR_BUILD="Build"
    STR_RUN="Run"
    STR_CLEAN="Clean"
    STR_REBUILD="ReBuild"
    STR_RUNBUILD="Build & Run"
    STR_ALL="All (Clean, Build, Run)"
    STR_RELOAD="Reload Shell"
    STR_CONFIG="Config"
    STR_EXIT="Exit"
    STR_IMG_LIST="Image files list..."
    STR_IMG_SELECT="Choose an image file..."
    STR_INPUT="Enter your choice: "
    STR_INVALID="Invalid choice!"
    STR_BUILDING="Building project..."
    STR_RUNNING="Running program..."
    STR_STOPPING="Stopping processes..."
    STR_SUCCESS="Success!"
    STR_FAILED="Failed!"
    STR_SAVE_CFG="Do you want to save the configuration?"
    STR_PRESS_KEY="Press any key to continue..."
fi

# -----------------------
# VARIABLES
# -----------------------
flag_exit=false
auto_build="disabled"
auto_run="disabled"
auto_clean="enabled"
img_num=1

# -----------------------
# CORE LOGIC
# -----------------------
function load_config() {
    if [ -f "$CONFIG_FILE" ]; then
        source "$CONFIG_FILE"
    else
        echo "auto_build=\"disabled\"" > "$CONFIG_FILE"
        echo "auto_run=\"disabled\"" >> "$CONFIG_FILE"
        echo "auto_clean=\"enabled\"" >> "$CONFIG_FILE"
        echo "img_num=\"1\"" >> "$CONFIG_FILE"
        source "$CONFIG_FILE"
    fi
    load_image_files
}

function save_config() {
    print_status "$GREEN" "$STR_SAVE_CFG (y/n): "
    read -e -p "   " save_choice
    if [[ "$save_choice" == "y" || "$save_choice" == "yes" ]]; then
        cat <<EOF > "$CONFIG_FILE"
auto_build="$auto_build"
auto_run="$auto_run"
auto_clean="$auto_clean"
img_num="$img_num"
EOF
        print_status "$GREEN" "$STR_SUCCESS"
    fi
}

function load_image_files() {
    img_files=($(ls "$IMAGE_DIR"/*.img 2>/dev/null))
    if [ ${#img_files[@]} -eq 0 ]; then
        image_file=""
        return
    fi
    [ "$img_num" -lt 1 ] || [ "$img_num" -gt "${#img_files[@]}" ] && img_num=1
    image_file="${img_files[$((img_num - 1))]}"
}

function show_info() {
    find_executable
    echo -e "${GREEN}   Shell Configuration${NC}"
    echo
    echo -e "   Working directory:          ${CYAN}$(pwd)${NC}"
    echo -e "   Project directory:          ${CYAN}$PROJECT_DIR${NC}"
    echo -e "   Executable directory:       ${CYAN}$EXE_DIR${NC}"
    echo -e "   Executable file:            ${YELLOW}$EXECUTABLE${NC}"
    echo -e "   Auto Build:                 ${ORANGE}$auto_build${NC}"
    echo -e "   Auto Run:                   ${ORANGE}$auto_run${NC}"
    echo -e "   Auto Clean:                 ${ORANGE}$auto_clean${NC}"
    echo -e "   Image Number:               ${INDIGO}$img_num${NC}"
    echo
}

function find_executable() {
    # Match application.exe or application
    EXECUTABLE=$(find "$EXE_DIR" -type f \( -name "application.exe" -o -name "application" \) 2>/dev/null | head -1)
}

function show_logo() {
    clear
    source "$BASE_DIR/DTH_logo.sh"
    echo -e "${VIOLET} ${ITALIC}   $STR_AUTHOR: DEESOL${NC}"
    echo -e "${CYAN} ${ITALIC}   $STR_DATE: $(date '+%Y-%m-%d %H:%M:%S')${NC}"
    echo
}

function show_menu() {
    echo -e "${GREEN}   $STR_MENU${NC}"
    echo -e "${ORANGE}   1. ${YELLOW}$STR_BUILD ${NC}"
    echo -e "${ORANGE}   2. ${YELLOW}$STR_RUN ${NC}"
    echo -e "${ORANGE}   3. ${YELLOW}$STR_CLEAN ${NC}"
    echo -e "${ORANGE}   4. ${YELLOW}$STR_REBUILD ${NC}"
    echo -e "${ORANGE}   5. ${YELLOW}$STR_RUNBUILD ${NC}"
    echo -e "${ORANGE}   6. ${YELLOW}$STR_ALL ${NC}"
    echo -e "${ORANGE}   7. ${YELLOW}$STR_RELOAD ${NC}"
    echo -e "${ORANGE}   8. ${YELLOW}$STR_CONFIG ${NC}"
    echo -e "${ORANGE}   9. ${YELLOW}$STR_EXIT ${NC}"
}

function build() {
    print_status "$GREEN" "$STR_BUILDING"
    (cd "$PROJECT_DIR" && $MAKE_CMD all)
    local ret=$?
    find_executable # Refresh after build
    if [ $ret -eq 0 ]; then
        print_status "$GREEN" "$STR_SUCCESS"
    else
        print_status "$RED" "$STR_FAILED"
    fi
}

function clean() {
    print_status "$GREEN" "Cleaning..."
    (cd "$PROJECT_DIR" && $MAKE_CMD clean) && print_status "$GREEN" "$STR_SUCCESS" || print_status "$RED" "$STR_FAILED"
}

function run() {
    print_status "$GREEN" "$STR_RUNNING"
    stop_processes "silent"
    
    find_executable # Refresh path
    
    if [ -f "$EXECUTABLE" ]; then
        "$EXECUTABLE" "$image_file" read-only
        local ret=$?
        [ $ret -ne 0 ] && print_status "$RED" "Exited with code: $ret"
    else
        print_status "$RED" "Executable not found! Please build first."
    fi
}

function stop_processes() {
    local silent=$1
    if [ -f "$PID_LOG" ]; then
        [ -z "$silent" ] && print_status "$ORANGE" "$STR_STOPPING"
        while IFS='|' read -r pid _; do
            if kill -0 "$pid" 2>/dev/null; then
                kill -TERM "$pid" >/dev/null 2>&1
            fi
        done < "$PID_LOG"
        rm -f "$PID_LOG"
    fi
}

function set_config() {
    # Simple config menu
    echo -e "${YELLOW}   1. Image Selection ($img_num)${NC}"
    echo -e "${YELLOW}   2. Auto Build ($auto_build)${NC}"
    echo -e "${YELLOW}   3. Return${NC}"
    read -e -p "Choice: " cfg_choice
    case $cfg_choice in
        1)
            load_image_files
            for i in "${!img_files[@]}"; do
                echo "   $((i+1)). ${img_files[$i]}"
            done
            read -e -p "$STR_IMG_SELECT: " ni
            if [[ "$ni" =~ ^[0-9]+$ ]] && [ "$ni" -ge 1 ] && [ "$ni" -le "${#img_files[@]}" ]; then
               img_num=$ni
               save_config
            fi
            ;;
        2)
            [[ "$auto_build" == "enabled" ]] && auto_build="disabled" || auto_build="enabled"
            save_config
            ;;
    esac
}

# -----------------------
# MAIN LOOP
# -----------------------
function main() {
    load_config
    while true; do
        show_logo
        show_info
        show_menu
        read -e -p "$STR_INPUT" choice
        case $choice in
            1) build ;;
            2) run ;;
            3) clean ;;
            4) clean; build ;;
            5) build; run ;;
            6) clean; build; run ;;
            7) exec bash "$BASE_DIR/DTH_SL.sh" ;;
            8) set_config ;;
            9) exit 0 ;;
            *) print_status "$RED" "$STR_INVALID" ;;
        esac
        print_status "$YELLOW" "$STR_PRESS_KEY"
        read -n 1 -s
    done
}

main
