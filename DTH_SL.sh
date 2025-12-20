#!/bin/bash
# Select language mode with english default VI or EN

#------------------
# Variables and Constants
#------------------
lang="EN"

if [ -f "$HOME/.dth_shell_lang" ]; then
    lang=$(cat "$HOME/.dth_shell_lang")
fi

clear

source ./DTH_logo.sh
echo
echo -e "${GREEN}Welcome to DTH Shell"
echo -e "${YELLOW}Please select language mode"
echo -e "${CYAN}1. Vietnamese"
echo -e "${CYAN}2. English${VIOLET}"
read -e -p "Select language mode (Vi or En): " lang
echo -e "${NC}"

case $lang in
    1 | VI | vi | Vi | vI | VIETNAM | Vietnam | vietnam | Vietnam | VN | vn | Vn | vN | VIE | Vie | vie | Vie | VIE-EN | Vie-En | vie-en | Vie-en | v | V)
        source ./DTH_VI.sh
        ;;
    2 | EN | en | En | eN | ENGLISH | English | english | English | ENG | eng | Eng | eNg | ENG-EN | Eng-En | eng-en | Eng-en | e | E)
        source ./DTH_EN.sh
        ;;
    *)
        echo "Default language mode is English"
        source ./DTH_EN.sh
        ;;
esac

sleep 1
