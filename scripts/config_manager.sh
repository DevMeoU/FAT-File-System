#!/bin/bash
# Shared configuration helpers for DTH shell scripts.

CONFIG_FILE="${CONFIG_FILE:-$BASE_DIR/shell_config.cfg}"

CONFIG_DEFAULT_AUTO_BUILD="disabled"
CONFIG_DEFAULT_AUTO_RUN="disabled"
CONFIG_DEFAULT_AUTO_CLEAN="enabled"
CONFIG_DEFAULT_IMG_NUM="1"
CONFIG_DEFAULT_MENU_THEME="number"
CONFIG_DEFAULT_COM_PORT=""
CONFIG_DEFAULT_AVAILABLE_COM_PORTS=""
CONFIG_DEFAULT_CHIP="auto"
CONFIG_DEFAULT_BAUD_RATE="115200"
CONFIG_DEFAULT_START_ADDRESS="0x0"
CONFIG_DEFAULT_SIZE="0xE70000"
CONFIG_DEFAULT_OUTPUT_FILE="project/images/storage_dump.bin"

function config_apply_defaults() {
    : "${auto_build:=$CONFIG_DEFAULT_AUTO_BUILD}"
    : "${auto_run:=$CONFIG_DEFAULT_AUTO_RUN}"
    : "${auto_clean:=$CONFIG_DEFAULT_AUTO_CLEAN}"
    : "${img_num:=$CONFIG_DEFAULT_IMG_NUM}"
    : "${menu_theme:=$CONFIG_DEFAULT_MENU_THEME}"
    : "${com_port:=$CONFIG_DEFAULT_COM_PORT}"
    : "${available_com_ports:=$CONFIG_DEFAULT_AVAILABLE_COM_PORTS}"
    : "${chip:=$CONFIG_DEFAULT_CHIP}"
    : "${baud_rate:=$CONFIG_DEFAULT_BAUD_RATE}"
    : "${start_address:=$CONFIG_DEFAULT_START_ADDRESS}"
    : "${size:=$CONFIG_DEFAULT_SIZE}"
    : "${output_file:=$CONFIG_DEFAULT_OUTPUT_FILE}"

    if [[ "$menu_theme" != "number" && "$menu_theme" != "arrow" ]]; then
        menu_theme="$CONFIG_DEFAULT_MENU_THEME"
    fi
}

function load_config() {
    config_apply_defaults

    if [ -f "$CONFIG_FILE" ]; then
        source "$CONFIG_FILE"
    fi

    config_apply_defaults
}

function config_write_value() {
    local key="$1"
    local value="$2"

    printf '%s=%q\n' "$key" "$value"
}

function config_write() {
    {
        printf '# DTH shell configuration\n'
        printf '# Build settings\n'
        config_write_value "auto_build" "$auto_build"
        config_write_value "auto_run" "$auto_run"
        config_write_value "auto_clean" "$auto_clean"
        config_write_value "img_num" "$img_num"
        config_write_value "menu_theme" "$menu_theme"
        printf '\n# Flash read settings\n'
        config_write_value "com_port" "$com_port"
        config_write_value "available_com_ports" "$available_com_ports"
        config_write_value "chip" "$chip"
        config_write_value "baud_rate" "$baud_rate"
        config_write_value "start_address" "$start_address"
        config_write_value "size" "$size"
        config_write_value "output_file" "$output_file"
    } > "$CONFIG_FILE"
}

function resolve_repo_path() {
    local path_value="$1"

    if [[ "$path_value" == /project/* ]]; then
        printf '%s%s\n' "$BASE_DIR" "$path_value"
    elif [[ "$path_value" =~ ^/ || "$path_value" =~ ^~ || "$path_value" =~ ^[A-Za-z]:[\\/].* ]]; then
        printf '%s\n' "$path_value"
    else
        printf '%s/%s\n' "$BASE_DIR" "$path_value"
    fi
}

function is_number_arg() {
    [[ "$1" =~ ^(0x[0-9A-Fa-f]+|[0-9]+)$ ]]
}

function list_com_ports() {
    local show_com_path="$PROJECT_DIR/tools/showCOMavailable.exe"
    local ports=""

    if [ -f "$show_com_path" ]; then
        ports=$("$show_com_path" 2>/dev/null | grep -Eo 'COM[0-9]+' | sort -u)
    fi

    if [ -z "$ports" ]; then
        ports=$(printf '%s\n' "$available_com_ports" | grep -Eo 'COM[0-9]+' | sort -u)
    fi

    printf '%s\n' "$ports" | grep -Eo 'COM[0-9]+' | sort -u
}

function list_available_chips() {
    local esptool_path="$PROJECT_DIR/tools/esptool.exe"
    local chips=""

    if [ -f "$esptool_path" ]; then
        chips=$("$esptool_path" --chip invalid read-flash --help 2>&1 \
            | grep -Eo "'[A-Za-z0-9_-]+'" \
            | tr -d "'" \
            | grep -Ev '^(--chip|-c|invalid)$')
    fi

    if [ -n "$chips" ]; then
        printf '%s\n' "$chips" | sort -u
        return 0
    fi

    printf '%s\n' auto esp8266 esp32 esp32s2 esp32s3 esp32c3 esp32c2 esp32c6 esp32c61 esp32c5 esp32e22 esp32h2 esp32h21 esp32p4 esp32h4 esp32s31
}

function validate_read_flash_args() {
    local selected_chip="$1"
    local selected_com_port="$2"
    local selected_baud_rate="$3"
    local selected_start_address="$4"
    local selected_size="$5"
    local selected_output_file="$6"

    if ! list_available_chips | grep -Fxq "$selected_chip"; then
        echo "Error: unsupported chip '$selected_chip'."
        return 1
    fi

    if [ -z "$selected_com_port" ]; then
        echo "Error: COM port is not set."
        return 1
    fi

    if [[ ! "$selected_baud_rate" =~ ^[0-9]+$ ]]; then
        echo "Error: baud_rate must be decimal number."
        return 1
    fi

    if ! is_number_arg "$selected_start_address"; then
        echo "Error: start_address must be decimal or hex, for example 0x0."
        return 1
    fi

    if ! is_number_arg "$selected_size"; then
        echo "Error: size must be decimal or hex, for example 0xE70000."
        return 1
    fi

    if [ -z "$selected_output_file" ]; then
        echo "Error: output_file is not set."
        return 1
    fi
}
