#!/bin/bash
# DTH Shell

#------------------
# Variables and Constants
#------------------
# Config & Log files
CUR_DIR="$(pwd)"
PROJECT_DIR="$CUR_DIR/project"
EXE_DIR="$PROJECT_DIR/build/bin"

# Phát hiện OS và đuôi EXE
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" || "$OS" == "Windows_NT" ]]; then
    EXE_EXT=".exe"
else
    EXE_EXT=""
fi

function find_executable() {
    # Khớp application.exe hoặc application (không đuôi)
    EXECUTABLE="$(find "$EXE_DIR" -type f \( -name "application.exe" -o -name "application" \) 2>/dev/null | head -1)"
}

CONFIG_FILE="shell_config.cfg"
PID_LOG=".processes.pid"

# Detect make command
if command -v mingw32-make >/dev/null 2>&1; then
    MAKE_CMD="mingw32-make"
else
    MAKE_CMD="make"
fi

# Flags
flag_exit=false

# Color codes
RED='\033[0;31m'
ORANGE='\033[0;33m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
INDIGO='\033[0;35m'
VIOLET='\033[1;35m'
CYAN='\033[0;36m'
NC='\033[0m'        # No Color
ITALIC='\033[3m'    # Italic text

#------------------
# Load and Save Config
#------------------
function load_config() {
    if [ -f "$CONFIG_FILE" ]; then
        source "$CONFIG_FILE"
    else
        # Nếu file không tồn tại, tạo file với giá trị mặc định
        echo "auto_build=\"disabled\"" > "$CONFIG_FILE"
        echo "auto_run=\"disabled\"" >> "$CONFIG_FILE"
        echo "auto_clean=\"enabled\"" >> "$CONFIG_FILE"
        echo "img_num=\"0\"" >> "$CONFIG_FILE"
        source "$CONFIG_FILE"
    fi
    load_image_files
}

function save_config() {
    echo -e "${GREEN}   Bạn có muốn lưu cấu hình không?${NC}"
    read -e -p "   Lưu cấu hình? (yes[y]/no[n]): " save_config

    case "$save_config" in
        "yes"|"y")
            cat <<EOF > "$CONFIG_FILE"
auto_build="$auto_build"
auto_run="$auto_run"
auto_clean="$auto_clean"
img_num="$img_num"
EOF
            echo -e "${GREEN}   Cấu hình đã được lưu thành công!${NC}"
            ;;
        *)
            echo -e "${RED}   Cấu hình không được lưu!${NC}"
            ;;
    esac
}

# -----------------------
# IMAGE FILE HANDLING
# -----------------------
function load_image_files() {
    img_files=($(ls ${PROJECT_DIR}/images/*.img 2>/dev/null))
    echo -e "${GREEN}   Đã tìm thấy ${#img_files[@]} file ảnh!${NC}"
    sleep 1
    
    if [ ${#img_files[@]} -eq 0 ]; then
        echo -e "${RED}   Không tìm thấy file ảnh nào!${NC}"
        image_file=""
        return
    fi

    if [ "$img_num" -lt 1 ] || [ "$img_num" -gt "${#img_files[@]}" ]; then
        img_num=1
    fi

    image_file="${img_files[$((img_num - 1))]}"
}

# -----------------------
# INIT SYSTEM
# -----------------------
function init_system() {
    clear
    [ -d "./project" ] || { echo -e "${RED}Thư mục project không tồn tại${NC}"; exit 1; }
    cd "./project" || { echo -e "${RED}Không thể truy cập thư mục project${NC}"; exit 1; }
    [ -f "$PID_LOG" ] || touch "$PID_LOG"
}

#------------------
# Display functions
#------------------
function show_logo() {
    clear
    source ../DTH_logo.sh
    echo -e "${VIOLET} ${ITALIC}   Tác giả: DEESOL${NC}"
    echo -e "${CYAN} ${ITALIC}   Ngày: $(date '+%Y-%m-%d %H:%M:%S')${NC}"
    echo -e "${NC}"
}

function show_config() {
    echo -e "${YELLOW}   Cấu hình Shell${NC}"
    echo
    echo -e "${GREEN}   Thư mục làm việc: ${VIOLET}${CUR_DIR}${NC}"
    echo -e "${GREEN}   Thư mục project:  ${VIOLET}${PROJECT_DIR}${NC}"
    echo -e "${GREEN}   Thư mục exe:      ${VIOLET}${EXE_DIR}${NC}"
    echo -e "${GREEN}   File exe:         ${VIOLET}${EXECUTABLE}${NC}"
    echo -e "${GREEN}   Tự động Build:    ${CYAN}$auto_build${NC}"
    echo -e "${GREEN}   Tự động Run:      ${CYAN}$auto_run${NC}"
    echo -e "${GREEN}   Tự động Clean:    ${CYAN}$auto_clean${NC}"
    echo -e "${GREEN}   Số Image:         ${ORANGE}$img_num${NC}"
}

function show_menu() {
    echo
    echo -e "${GREEN}   Menu${NC}"
    echo
    echo -e "${ORANGE}   1. ${YELLOW}Build       ${NC}- Build project"
    echo -e "${ORANGE}   2. ${YELLOW}Run         ${NC}- Run project"
    echo -e "${ORANGE}   3. ${YELLOW}Clean       ${NC}- Clean project"
    echo -e "${ORANGE}   4. ${YELLOW}ReBuild     ${NC}- ReBuild project"
    echo -e "${ORANGE}   5. ${YELLOW}RunBuild    ${NC}- Run và Build project"
    echo -e "${ORANGE}   6. ${YELLOW}All         ${NC}- Run, Build và Clean project"
    echo -e "${ORANGE}   7. ${YELLOW}Reload      ${NC}- Reload terminal"
    echo -e "${ORANGE}   8. ${YELLOW}Config      ${NC}- Thiết lập cấu hình"
    echo -e "${ORANGE}   9. ${YELLOW}Thoát       ${NC}- Thoát shell"

}

function show_img_files() {
    echo -e "${GREEN}   Danh sách file ảnh...${NC}"
    
    img_files=()
    counter=1
    while IFS= read -r img_file; do
        img_files+=("$img_file")
        if [ "$counter" -eq "$img_num" ]; then
            echo -e "${CYAN} → $counter. $img_file ${NC}"
        else
            echo "   $counter. $img_file"
        fi
        ((counter++))
    done < <(ls images/*.img 2>/dev/null | sort)

    if [ "${#img_files[@]}" -eq 0 ]; then
        echo -e "${RED}   Không tìm thấy file ảnh nào!${NC}"
    fi
}

function show_menu_config() {
    echo -e "${GREEN}   Menu Cấu Hình${NC}"
    echo "   1. Thiết lập số image"
    echo "   2. Thiết lập auto build"
    echo "   3. Thiết lập auto run"
    echo "   4. Thiết lập auto clean"
    echo "   5. Quay lại menu chính"
}

#------------------
# User input functions
#------------------
function handle_user_choice() {
    local choice
    read -e -p "Nhập lựa chọn của bạn: " choice
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
        9) flag_exit=true; echo -e "${RED}   Đang thoát shell...${NC}"; return ;;  # Thoát ngay lập tức
        *) echo -e "${RED}Lựa chọn không hợp lệ, vui lòng thử lại!${NC}" ;;
    esac
    press_any_key;
}

function press_any_key() {
    read -n 1 -s -r -p "Press any key to continue..."
    echo  # in dòng mới sau khi nhấn phím
}

function handle_config_choice() {
    local choice
    read -e -p "   Nhập lựa chọn của bạn: " choice
    case $choice in
        1) set_img_num ;;
        2) set_auto_build ;;
        3) set_auto_run ;;
        4) set_auto_clean ;;
        5) return 5;;
        *) echo -e "${RED}   Lựa chọn không hợp lệ!${NC}" ;;
    esac
    return $choice
}

#------------------
# Project functions
#------------------
function build() {
    echo -e "${GREEN}   Đang build project...${NC}"
    if $MAKE_CMD all; then
        echo -e "${GREEN}   Build hoàn tất!${NC}"
    else
        echo -e "${RED}   Build thất bại!${NC}"
    fi
}

function run() {
    echo -e "\n${GREEN}   Khởi chạy chương trình...${NC}"
    stop_processes "silent"  # Dừng tiến trình cũ trước khi chạy mới
    find_executable  # Cập nhật đường dẫn file thực thi trước khi kiểm tra
    if [ -f "$EXECUTABLE" ]; then
        "$EXECUTABLE" "$image_file" read-only  # Chạy foreground, không dùng `&`
        local app_exit_code=$?  # Lấy mã thoát của ứng dụng ngay sau khi kết thúc

        # Kiểm tra exit code đặc biệt
        if [ $app_exit_code -eq 42 ]; then
            echo -e "${CYAN}   Ứng dụng đã thoát, quay lại DTH Shell${NC}"
        elif [ $app_exit_code -ne 0 ]; then
            echo -e "${RED}   Ứng dụng đã thoát với lỗi: $app_exit_code${NC}"
        fi
    else
        echo -e "${RED}   Lỗi: Không tìm thấy file application!${NC}"
    fi
}

function stop_processes() {
    local silent=$1
    [ -z "$silent" ] && echo -e "\n${ORANGE}   Đang dừng các tiến trình...${NC}"
    
    if [ -f "$PID_LOG" ]; then
        while IFS='|' read -r pid _; do
            if kill -0 "$pid" 2>/dev/null; then
                kill -TERM "$pid" >/dev/null 2>&1
                [ -z "$silent" ] && echo -e "   ${RED}Đã dừng PID: $pid${NC}"
            fi
        done < "$PID_LOG"
        rm -f "$PID_LOG"
    fi
}

function get_run_pid() {
    if [ -n "$run_pid" ]; then
        echo -e "${CYAN}   PID của project đang chạy: $run_pid${NC}"
    else
        echo -e "${RED}   Không tìm thấy tiến trình đang chạy!${NC}"
    fi
}

function clean() {
    echo -e "${GREEN}   Đang clean project...${NC}"
    if $MAKE_CMD clean; then
        echo -e "${GREEN}   Clean hoàn tất!${NC}"
    else
        echo -e "${RED}   Clean thất bại!${NC}"
    fi
}

function reload() {
    exec "$SHELL"
}

#------------------
# Configuration functions
#------------------
function set_img_num() {
    show_img_files
    echo -e "${YELLOW}   Chọn file ảnh...${NC}"
    read -e -p "   Nhập số (1-${#img_files[@]}): " input_num

    if [[ "$input_num" =~ ^[0-9]+$ ]] && [ "$input_num" -ge 1 ] && [ "$input_num" -le "${#img_files[@]}" ]; then
        img_num=$input_num
        image_file="${img_files[$((img_num - 1))]}"
        echo -e "${GREEN}   Đã chọn file ảnh: $image_file${NC}"
    else
        echo -e "${RED}   Số không hợp lệ!${NC}"
    fi
    save_config
}

function set_auto_build() {
    echo -e "${YELLOW}   Thiết lập auto build...${NC}"
    read -e -p "   Bật auto build? (enabled[e]/disabled[d]): " input_value
    case "$input_value" in
        "enabled"|"e") auto_build="enabled" ;;
        "disabled"|"d") auto_build="disabled" ;;
        *)
            echo -e "${RED}   Nhập không hợp lệ!${NC}"
            set_auto_build
            return
            ;;
    esac
    echo -e "${GREEN}   Auto build được đặt thành $auto_build${NC}"
}

function set_auto_run() {
    echo -e "${YELLOW}   Thiết lập auto run...${NC}"
    read -e -p "   Bật auto run? (enabled[e]/disabled[d]): " input_value
    case "$input_value" in
        "enabled"|"e") auto_run="enabled" ;;
        "disabled"|"d") auto_run="disabled" ;;
        *)
            echo -e "${RED}   Nhập không hợp lệ!${NC}"
            set_auto_run
            return
            ;;
    esac
    echo -e "${GREEN}   Auto run được đặt thành $auto_run${NC}"
}

function set_auto_clean() {
    echo -e "${YELLOW}   Thiết lập auto clean...${NC}"
    read -e -p "   Bật auto clean? (enabled[e]/disabled[d]): " input_value
    case "$input_value" in
        "enabled"|"e") auto_clean="enabled" ;;
        "disabled"|"d") auto_clean="disabled" ;;
        *)
            echo -e "${RED}   Nhập không hợp lệ!${NC}"
            set_auto_clean
            return
            ;;
    esac
    echo -e "${GREEN}   Auto clean được đặt thành $auto_clean${NC}"
}

function set_config() {
    while true; do
        show_logo
        show_config
        echo
        show_menu_config
        handle_config_choice
        choice=$?
        if [ "$choice" == "5" ]; then
            break
        fi
    done
}

#------------------
# Main shell function
#------------------
function main() {
    init_system
    load_config
    
    while true; do
        EXECUTABLE=$(find "$EXE_DIR" -type f -name '*.exe' 2>/dev/null | head -1)
        clear
        show_logo
        show_config
        show_menu
        handle_user_choice
        sleep 2;

        if [ "$flag_exit" == "true" ]; then
            break
        fi

        if [ "$auto_clean" == "enabled" ]; then
            clean
        fi

        if [ "$auto_build" == "enabled" ]; then
            build
        fi

        if [ "$auto_run" == "enabled" ]; then
            run
        fi

    done
}

# Khởi động shell
main

