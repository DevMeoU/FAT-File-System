#!/bin/bash
# read_flash.sh - ESP flash read helpers.

SOURCE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SOURCE_DIR/common_utils.sh"
source "$SOURCE_DIR/config_manager.sh"

ESPTOOL_PATH="$PROJECT_DIR/tools/esptool.exe"

function read_flash() {
    load_config

    local arg_chip="${1:-$chip}"
    local arg_com_port="${2:-$com_port}"
    local arg_baud_rate="${3:-$baud_rate}"
    local arg_start_address="${4:-$start_address}"
    local arg_size="${5:-$size}"
    local arg_output_file="${6:-$output_file}"
    local resolved_output_file
    local output_dir

    if [ ! -f "$ESPTOOL_PATH" ]; then
        echo "Error: esptool.exe not found at $ESPTOOL_PATH"
        return 1
    fi

    if ! validate_read_flash_args "$arg_chip" "$arg_com_port" "$arg_baud_rate" "$arg_start_address" "$arg_size" "$arg_output_file"; then
        return 1
    fi

    resolved_output_file="$(resolve_repo_path "$arg_output_file")"
    output_dir="$(dirname "$resolved_output_file")"
    if [ ! -d "$output_dir" ]; then
        echo "Error: output directory not found: $output_dir"
        return 1
    fi

    "$ESPTOOL_PATH" --chip "$arg_chip" -p "$arg_com_port" -b "$arg_baud_rate" read_flash "$arg_start_address" "$arg_size" "$resolved_output_file"
}

function read_flash_from_config() {
    read_flash
}

if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
    read_flash "$@"
fi
