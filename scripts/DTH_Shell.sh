#!/bin/bash
# DTH_Shell.sh - Unified Shell for FAT Project
# Author: DEESOL

SOURCE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SOURCE_DIR/common_utils.sh"
source "$SOURCE_DIR/config_manager.sh"
source "$SOURCE_DIR/read_flash.sh"

LANG_MODE=${1:-"EN"}
PID_LOG="$PROJECT_DIR/.processes.pid"

# -----------------------
# LOCALIZATION STRINGS
# -----------------------
if [[ "$LANG_MODE" == "VI" ]]; then
    STR_AUTHOR="Tac gia"
    STR_DATE="Ngay"
    STR_MENU="Menu"
    STR_BUILD="Build"
    STR_RUN="Chay"
    STR_CLEAN="Clean"
    STR_REBUILD="ReBuild"
    STR_RUNBUILD="Build va Chay"
    STR_ALL="All (Clean, Build, Run)"
    STR_RELOAD="Tai lai Shell"
    STR_CONFIG="Cau hinh"
    STR_LOAD_FLASH="Load Flash"
    STR_EXIT="Thoat"
    STR_MENU_THEME="Kieu menu"
    STR_NUMBER_THEME="Chon bang so"
    STR_ARROW_THEME="Len/xuong + Enter"
    STR_IMG_SELECT="Chon file anh"
    STR_INPUT="Nhap lua chon cua ban: "
    STR_INVALID="Lua chon khong hop le!"
    STR_BUILDING="Dang build project..."
    STR_RUNNING="Khoi chay chuong trinh..."
    STR_STOPPING="Dang dung cac tien trinh..."
    STR_SUCCESS="Thanh cong!"
    STR_FAILED="That bai!"
    STR_SAVE_CFG="Ban co muon luu cau hinh khong?"
    STR_PRESS_KEY="Nhan phim bat ky de tiep tuc..."
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
    STR_LOAD_FLASH="Load Flash"
    STR_EXIT="Exit"
    STR_MENU_THEME="Menu Theme"
    STR_NUMBER_THEME="Number selection"
    STR_ARROW_THEME="Up/Down + Enter"
    STR_IMG_SELECT="Choose an image file"
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

STR_COM_SELECT="Choose COM port"
STR_COM_NONE="No available COM ports found."
STR_COM_SELECTED="Selected COM port"
STR_CHIP_SELECT="Choose chip"
STR_CHIP_NONE="No available chips found."
STR_CHIP_SELECTED="Selected chip"
STR_ARROW_HINT="Use Up/Down, Home/End, Enter. Press q or Esc to cancel."

flag_exit=false
MENU_CHOICE=""
OPTION_CHOICE=""

function refresh_screen() {
    clear
}

function render_main_header() {
    show_logo
    show_info
}

function render_arrow_block() {
    local title="$1"
    local labels_name="$2"
    local selected="$3"
    local -n labels_ref="$labels_name"

    printf '\033[2K\r'
    echo -e "${GREEN}   $title${NC}"
    printf '\033[2K\r'
    echo -e "${CYAN}   $STR_ARROW_HINT${NC}"
    printf '\033[2K\r'
    echo

    for i in "${!labels_ref[@]}"; do
        printf '\033[2K\r'
        if [ "$i" -eq "$selected" ]; then
            echo -e "${ORANGE} > ${YELLOW}${labels_ref[$i]}${NC}"
        else
            echo -e "   ${labels_ref[$i]}"
        fi
    done
}

function redraw_arrow_block() {
    local title="$1"
    local labels_name="$2"
    local selected="$3"
    local -n labels_ref="$labels_name"
    local block_lines=$((${#labels_ref[@]} + 3))

    printf '\033[%dA' "$block_lines"
    render_arrow_block "$title" "$labels_name" "$selected"
}

function read_arrow_choice() {
    local title="$1"
    local labels_name="$2"
    local values_name="$3"
    local render_func="${4:-}"
    local -n labels_ref="$labels_name"
    local -n values_ref="$values_name"
    local selected=0
    local old_selected=0
    local key=""
    local key_rest=""

    MENU_CHOICE=""

    if [ -n "$render_func" ]; then
        "$render_func"
    else
        refresh_screen
    fi
    render_arrow_block "$title" "$labels_name" "$selected"
    tput civis 2>/dev/null || true

    while true; do
        old_selected="$selected"

        IFS= read -rsn1 key
        case "$key" in
            "")
                MENU_CHOICE="${values_ref[$selected]}"
                tput cnorm 2>/dev/null || true
                return 0
                ;;
            q|Q)
                tput cnorm 2>/dev/null || true
                return 1
                ;;
            $'\x1b')
                key_rest=""
                IFS= read -rsn3 -t 0.05 key_rest || true
                case "$key_rest" in
                    "[A")
                        selected=$((selected - 1))
                        [ "$selected" -lt 0 ] && selected=$((${#labels_ref[@]} - 1))
                        ;;
                    "[B")
                        selected=$((selected + 1))
                        [ "$selected" -ge "${#labels_ref[@]}" ] && selected=0
                        ;;
                    "[H"|"[1~")
                        selected=0
                        ;;
                    "[F"|"[4~")
                        selected=$((${#labels_ref[@]} - 1))
                        ;;
                    "")
                        tput cnorm 2>/dev/null || true
                        return 1
                        ;;
                esac
                ;;
        esac

        if [ "$selected" -ne "$old_selected" ]; then
            redraw_arrow_block "$title" "$labels_name" "$selected"
        fi
    done
}

function choose_from_options() {
    local title="$1"
    local labels_name="$2"
    local values_name="$3"
    local prompt="$4"
    local -n labels_ref="$labels_name"
    local -n values_ref="$values_name"
    local ni=""

    OPTION_CHOICE=""

    if [ "$menu_theme" == "arrow" ]; then
        if read_arrow_choice "$title" "$labels_name" "$values_name"; then
            OPTION_CHOICE="$MENU_CHOICE"
            return 0
        fi
        return 1
    fi

    refresh_screen
    echo -e "${GREEN}   $title${NC}"
    for i in "${!labels_ref[@]}"; do
        echo "   $((i+1)). ${labels_ref[$i]}"
    done

    read -e -p "$prompt" ni
    if [[ "$ni" =~ ^[0-9]+$ ]] && [ "$ni" -ge 1 ] && [ "$ni" -le "${#labels_ref[@]}" ]; then
        OPTION_CHOICE="${values_ref[$((ni - 1))]}"
        return 0
    fi

    print_status "$RED" "$STR_INVALID"
    return 1
}

function save_config() {
    print_status "$GREEN" "$STR_SAVE_CFG (y/n): "
    read -e -p "   " save_choice
    if [[ "$save_choice" == "y" || "$save_choice" == "yes" ]]; then
        config_write
        print_status "$GREEN" "$STR_SUCCESS"
    fi
}

function load_image_files() {
    img_files=($(LC_COLLATE=C ls "$IMAGE_DIR"/*.img "$IMAGE_DIR"/*.bin 2>/dev/null))
    if [ ${#img_files[@]} -eq 0 ]; then
        image_file=""
        return
    fi

    if [[ ! "$img_num" =~ ^[0-9]+$ ]] || [ "$img_num" -lt 1 ] || [ "$img_num" -gt "${#img_files[@]}" ]; then
        img_num=1
    fi

    image_file="${img_files[$((img_num - 1))]}"
}

function find_executable() {
    EXECUTABLE=$(find "$EXE_DIR" -type f \( -name "application.exe" -o -name "application" \) 2>/dev/null | head -1)
}

function show_info() {
    find_executable
    local image_display="not selected"
    if [ -n "$image_file" ]; then
        image_display="$(basename "$image_file")"
    fi

    echo -e "${GREEN}   Shell Configuration${NC}"
    echo
    echo -e "${YELLOW}   Paths${NC}"
    echo -e "   Working directory:          ${CYAN}$(pwd)${NC}"
    echo -e "   Project directory:          ${CYAN}$PROJECT_DIR${NC}"
    echo -e "   Executable directory:       ${CYAN}$EXE_DIR${NC}"
    echo -e "   Executable file:            ${YELLOW}$EXECUTABLE${NC}"
    echo -e "   Config file:                ${CYAN}$CONFIG_FILE${NC}"
    echo
    echo -e "${YELLOW}   Build${NC}"
    echo -e "   Auto Build:                 ${ORANGE}$auto_build${NC}"
    echo -e "   Auto Run:                   ${ORANGE}$auto_run${NC}"
    echo -e "   Auto Clean:                 ${ORANGE}$auto_clean${NC}"
    echo -e "   Image Number:               ${INDIGO}$img_num${NC}"
    echo -e "   Image File:                 ${CYAN}$image_display${NC}"
    echo -e "   Menu Theme:                 ${CYAN}$menu_theme${NC}"
    echo
    echo -e "${YELLOW}   Flash Read${NC}"
    echo -e "   COM Port:                   ${CYAN}${com_port:-not selected}${NC}"
    echo -e "   Available COM Ports:        ${CYAN}${available_com_ports:-not cached}${NC}"
    echo -e "   Chip:                       ${CYAN}$chip${NC}"
    echo -e "   Baud Rate:                  ${CYAN}$baud_rate${NC}"
    echo -e "   Start Address:              ${CYAN}$start_address${NC}"
    echo -e "   Size:                       ${CYAN}$size${NC}"
    echo -e "   Output File:                ${CYAN}$output_file${NC}"
    echo
}

function show_logo() {
    refresh_screen
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
    echo -e "${ORANGE}   9. ${YELLOW}$STR_LOAD_FLASH ${NC}"
    echo -e "${ORANGE}   10. ${YELLOW}$STR_EXIT ${NC}"
}

function build() {
    refresh_screen
    print_status "$GREEN" "$STR_BUILDING"
    (cd "$PROJECT_DIR" && $MAKE_CMD all IMAGE_NUM="$img_num")
    local ret=$?
    find_executable
    if [ $ret -eq 0 ]; then
        print_status "$GREEN" "$STR_SUCCESS"
    else
        print_status "$RED" "$STR_FAILED"
    fi
}

function clean() {
    refresh_screen
    print_status "$GREEN" "Cleaning..."
    (cd "$PROJECT_DIR" && $MAKE_CMD clean IMAGE_NUM="$img_num") && print_status "$GREEN" "$STR_SUCCESS" || print_status "$RED" "$STR_FAILED"
}

function run() {
    refresh_screen
    print_status "$GREEN" "$STR_RUNNING"
    stop_processes "silent"
    find_executable

    if [ -f "$EXECUTABLE" ]; then
        echo -e "   Executing: ${CYAN}$EXECUTABLE${NC} ${YELLOW}$image_file${NC} read-only"
        "$EXECUTABLE" "$image_file" read-only
        local ret=$?
        [ $ret -ne 0 ] && print_status "$RED" "Exited with code: $ret"
    else
        print_status "$RED" "Executable not found! Please build first."
    fi
}

function load_flash() {
    local resolved_output_file

    refresh_screen
    load_config
    resolved_output_file="$(resolve_repo_path "$output_file")"

    echo -e "${GREEN}   Loading flash to file${NC}"
    echo -e "   Chip:          ${CYAN}$chip${NC}"
    echo -e "   COM Port:      ${CYAN}${com_port:-not selected}${NC}"
    echo -e "   Baud Rate:     ${CYAN}$baud_rate${NC}"
    echo -e "   Start Address: ${CYAN}$start_address${NC}"
    echo -e "   Size:          ${CYAN}$size${NC}"
    echo -e "   Output File:   ${CYAN}$resolved_output_file${NC}"

    if read_flash_from_config; then
        print_status "$GREEN" "$STR_SUCCESS"
        load_image_files
    else
        print_status "$RED" "$STR_FAILED"
        return 1
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

function select_image_file() {
    local -a image_labels=()
    local -a image_values=()

    refresh_screen
    load_image_files

    if [ ${#img_files[@]} -eq 0 ]; then
        print_status "$RED" "No image files found."
        return 1
    fi

    for i in "${!img_files[@]}"; do
        image_labels+=("${img_files[$i]}")
        image_values+=("$((i + 1))")
    done

    if choose_from_options "$STR_IMG_SELECT" image_labels image_values "$STR_IMG_SELECT: "; then
        img_num="$OPTION_CHOICE"
        load_image_files
        save_config
    else
        return 1
    fi
}

function select_com_port() {
    local -a com_ports=()
    local -a com_values=()
    local port

    refresh_screen
    while IFS= read -r port; do
        [ -n "$port" ] && com_ports+=("$port")
    done < <(list_com_ports)

    if [ ${#com_ports[@]} -eq 0 ]; then
        print_status "$RED" "$STR_COM_NONE"
        return 1
    fi

    available_com_ports="${com_ports[*]}"
    com_values=("${com_ports[@]}")

    if choose_from_options "$STR_COM_SELECT" com_ports com_values "Select COM port: "; then
        com_port="$OPTION_CHOICE"
        print_status "$GREEN" "$STR_COM_SELECTED: $com_port"
        save_config
    else
        return 1
    fi
}

function select_chip() {
    local -a chips=()
    local -a chip_values=()
    local available_chip

    refresh_screen
    while IFS= read -r available_chip; do
        [ -n "$available_chip" ] && chips+=("$available_chip")
    done < <(list_available_chips)

    if [ ${#chips[@]} -eq 0 ]; then
        print_status "$RED" "$STR_CHIP_NONE"
        return 1
    fi

    chip_values=("${chips[@]}")

    if choose_from_options "$STR_CHIP_SELECT" chips chip_values "Select chip: "; then
        chip="$OPTION_CHOICE"
        print_status "$GREEN" "$STR_CHIP_SELECTED: $chip"
        save_config
    else
        return 1
    fi
}

function select_menu_theme() {
    local -a theme_labels=("$STR_NUMBER_THEME" "$STR_ARROW_THEME")
    local -a theme_values=("number" "arrow")

    if choose_from_options "$STR_MENU_THEME" theme_labels theme_values "$STR_MENU_THEME: "; then
        menu_theme="$OPTION_CHOICE"
        save_config
    fi
}

function set_config_value() {
    local var_name="$1"
    local label="$2"
    local current_value="$3"
    local pattern="$4"
    local new_value

    refresh_screen
    read -e -p "$label [$current_value]: " new_value
    [ -z "$new_value" ] && new_value="$current_value"

    if [ -n "$pattern" ] && [[ ! "$new_value" =~ $pattern ]]; then
        print_status "$RED" "$STR_INVALID"
        return 1
    fi

    if [[ "$new_value" == *\"* ]]; then
        print_status "$RED" "$STR_INVALID"
        return 1
    fi

    printf -v "$var_name" '%s' "$new_value"
    save_config
}

function toggle_config_value() {
    local var_name="$1"
    local current_value="${!var_name}"

    if [ "$current_value" == "enabled" ]; then
        printf -v "$var_name" '%s' "disabled"
    else
        printf -v "$var_name" '%s' "enabled"
    fi

    save_config
}

function show_config_menu() {
    refresh_screen
    echo -e "${GREEN}   $STR_CONFIG${NC}"
    echo -e "${YELLOW}   Build${NC}"
    echo -e "   1. Image Selection ($img_num)"
    echo -e "   2. Auto Build ($auto_build)"
    echo -e "   3. Auto Run ($auto_run)"
    echo -e "   4. Auto Clean ($auto_clean)"
    echo -e "   5. $STR_MENU_THEME ($menu_theme)"
    echo
    echo -e "${YELLOW}   Flash Read${NC}"
    echo -e "   6. COM Port (${com_port:-not selected})"
    echo -e "   7. Chip ($chip)"
    echo -e "   8. Baud Rate ($baud_rate)"
    echo -e "   9. Start Address ($start_address)"
    echo -e "   10. Size ($size)"
    echo -e "   11. Output File ($output_file)"
    echo
    echo -e "   0. Return"
}

function read_config_choice() {
    cfg_choice=""

    if [ "$menu_theme" == "arrow" ]; then
        local -a config_labels=(
            "Return"
            "Image Selection ($img_num)"
            "Auto Build ($auto_build)"
            "Auto Run ($auto_run)"
            "Auto Clean ($auto_clean)"
            "$STR_MENU_THEME ($menu_theme)"
            "COM Port (${com_port:-not selected})"
            "Chip ($chip)"
            "Baud Rate ($baud_rate)"
            "Start Address ($start_address)"
            "Size ($size)"
            "Output File ($output_file)"
        )
        local -a config_values=(0 1 2 3 4 5 6 7 8 9 10 11)

        if read_arrow_choice "$STR_CONFIG" config_labels config_values; then
            cfg_choice="$MENU_CHOICE"
            return 0
        fi
        return 1
    fi

    show_config_menu
    read -e -p "Choice: " cfg_choice
}

function set_config() {
    while true; do
        read_config_choice || continue

        case $cfg_choice in
            0) return ;;
            1) select_image_file ;;
            2) toggle_config_value "auto_build" ;;
            3) toggle_config_value "auto_run" ;;
            4) toggle_config_value "auto_clean" ;;
            5) select_menu_theme ;;
            6) select_com_port ;;
            7) select_chip ;;
            8) set_config_value "baud_rate" "Baud rate" "$baud_rate" '^[0-9]+$' ;;
            9) set_config_value "start_address" "Start address" "$start_address" '^(0x[0-9A-Fa-f]+|[0-9]+)$' ;;
            10) set_config_value "size" "Size" "$size" '^(0x[0-9A-Fa-f]+|[0-9]+)$' ;;
            11) set_config_value "output_file" "Output file" "$output_file" '^.+$' ;;
            *) print_status "$RED" "$STR_INVALID" ;;
        esac
    done
}

function read_main_choice() {
    choice=""

    if [ "$menu_theme" == "arrow" ]; then
        local -a main_labels=(
            "$STR_BUILD"
            "$STR_RUN"
            "$STR_CLEAN"
            "$STR_REBUILD"
            "$STR_RUNBUILD"
            "$STR_ALL"
            "$STR_RELOAD"
            "$STR_CONFIG"
            "$STR_LOAD_FLASH"
            "$STR_EXIT"
        )
        local -a main_values=(1 2 3 4 5 6 7 8 9 10)

        if read_arrow_choice "$STR_MENU" main_labels main_values render_main_header; then
            choice="$MENU_CHOICE"
            return 0
        fi
        return 1
    fi

    show_logo
    show_info
    show_menu
    read -e -p "$STR_INPUT" choice
}

function main() {
    load_config
    load_image_files

    while true; do
        read_main_choice || continue

        case $choice in
            1) build ;;
            2) run ;;
            3) clean ;;
            4) clean; build ;;
            5) build; run ;;
            6) clean; build; run ;;
            7) exec bash "$BASE_DIR/DTH_SL.sh" ;;
            8) set_config ;;
            9) load_flash ;;
            10) exit 0 ;;
            *) print_status "$RED" "$STR_INVALID" ;;
        esac

        print_status "$YELLOW" "$STR_PRESS_KEY"
        read -n 1 -s
    done
}

main
