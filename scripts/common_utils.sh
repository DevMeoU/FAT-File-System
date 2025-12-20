#!/bin/bash
# common_utils.sh - Centralized utility script
# Author: DEESOL

# -----------------------
# COLOR CODES
# -----------------------
export RED='\033[0;31m'
export ORANGE='\033[0;33m'
export YELLOW='\033[1;33m'
export GREEN='\033[0;32m'
export BLUE='\033[0;34m'
export INDIGO='\033[0;35m'
export VIOLET='\033[1;35m'
export CYAN='\033[1;36m'
export NC='\033[0m'        # No Color
export ITALIC='\033[3m'    # Italic text

# -----------------------
# PATH DEFINITIONS
# -----------------------
export BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
export PROJECT_DIR="$BASE_DIR/project"
export EXE_DIR="$PROJECT_DIR/build/bin"
export IMAGE_DIR="$PROJECT_DIR/images"
export SCRIPTS_DIR="$BASE_DIR/scripts"

# -----------------------
# ENVIRONMENT DETECTION
# -----------------------
if command -v mingw32-make >/dev/null 2>&1; then
    export MAKE_CMD="mingw32-make"
else
    export MAKE_CMD="make"
fi

# Detect OS
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" || "$OS" == "Windows_NT" ]]; then
    export PLATFORM="windows"
    export EXE_EXT=".exe"
else
    export PLATFORM="linux"
    export EXE_EXT=""
fi

# -----------------------
# COMMON FUNCTIONS
# -----------------------
function print_status() {
    local color=$1
    local msg=$2
    echo -e "${color}   $msg${NC}"
}

function check_file() {
    if [ ! -f "$1" ]; then
        print_status "$RED" "Error: File $1 not found."
        return 1
    fi
    return 0
}

function check_dir() {
    if [ ! -d "$1" ]; then
        print_status "$RED" "Error: Directory $1 not found."
        return 1
    fi
    return 0
}
