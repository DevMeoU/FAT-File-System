#!/bin/bash
# DTH_SL.sh - Project Entry Point & Language Selector

# Determine project root
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Load saved language preference
LANG_FILE="$HOME/.dth_shell_lang"
[[ -f "$LANG_FILE" ]] && selected_lang=$(cat "$LANG_FILE") || selected_lang="EN"

clear
source ./DTH_logo.sh
echo
echo -e "\033[0;32m   Welcome to DTH Shell"
echo -e "\033[1;33m   Please select language mode"
echo -e "\033[0;36m   1. Vietnamese"
echo -e "\033[0;36m   2. English\033[0m"
read -e -p "   Selection (1/2): " lang_choice

case $lang_choice in
    1)
        selected_lang="VI"
        ;;
    2)
        selected_lang="EN"
        ;;
    *)
        echo "Defaulting to saved/English..."
        ;;
esac

echo "$selected_lang" > "$LANG_FILE"

# Launch the unified shell
exec bash "$BASE_DIR/scripts/DTH_Shell.sh" "$selected_lang"
