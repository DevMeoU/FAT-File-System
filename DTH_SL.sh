# Determine project root
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Load saved language preference
LANG_FILE="$HOME/.dth_shell_lang"
[[ -f "$LANG_FILE" ]] && selected_lang=$(cat "$LANG_FILE") || selected_lang="EN"

clear
source ./DTH_logo.sh
echo
echo -e "${GREEN}   Welcome to DTH Shell"
echo -e "${YELLOW}   Please select language mode"
echo -e "${CYAN}   1. Vietnamese"
echo -e "${CYAN}   2. English${NC}"
read -e -p "   Selection (1/2): " lang_choice

case $lang_choice in
    1)
        echo "VI" > "$LANG_FILE"
        source ./DTH_VI.sh
        ;;
    2)
        echo "EN" > "$LANG_FILE"
        source ./DTH_EN.sh
        ;;
    *)
        echo "Defaulting to English..."
        source ./DTH_EN.sh
        ;;
esac


