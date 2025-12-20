/**
 * @file cli_helper.c
 * @brief Implementation of CLI helper utility (Portable version)
 */

#include "cli_helper.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include "../log/print_color.h"

/* Portable console input headers */
#ifdef _WIN32
    #include <windows.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

/**
 * Portable getch equivalent
 */
static int portable_getch(void) {
#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));

    INPUT_RECORD ir;
    DWORD read;
    int ch = 0;

    while (1) {
        ReadConsoleInput(hStdin, &ir, 1, &read);
        if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown) {
            ch = ir.Event.KeyEvent.uChar.AsciiChar;
            if (ch == 0) {
                /* Special key handling */
                continue;
            }
            break;
        }
    }

    SetConsoleMode(hStdin, mode);
    return ch;
#else
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
#endif
}

static const char* s_commands[] = {
    "ls", "cd", "cat", "evidence", "cls", "clear", "help", "exit", "quit", NULL
};

static void handle_tab_completion(Middleware* middleware, char* buffer, int* pos, size_t size) {
    char completion_prefix[256];
    strncpy(completion_prefix, buffer, *pos);
    completion_prefix[*pos] = '\0';

    char* last_space = strrchr(completion_prefix, ' ');
    char* prefix_to_match;
    int is_command = 0;

    if (last_space == NULL) {
        prefix_to_match = completion_prefix;
        is_command = 1;
    } else {
        prefix_to_match = last_space + 1;
        is_command = 0;
    }

    size_t prefix_len = strlen(prefix_to_match);
    if (prefix_len == 0 && !is_command) return;

    const char* matches[64];
    int match_count = 0;

    if (is_command) {
        for (int i = 0; s_commands[i] != NULL; i++) {
            if (strncmp(prefix_to_match, s_commands[i], prefix_len) == 0) {
                matches[match_count++] = s_commands[i];
            }
        }
    } else {
        /* File/Path completion */
        FileNode* current = middleware->current_directory;
        if (current) {
            /* Ensure children are populated */
            fat_driver_list_directory(middleware->fat_driver, current);
            
            FileNode* node = current->children;
            while (node && match_count < 64) {
                if (strncasecmp(prefix_to_match, node->name, prefix_len) == 0) {
                    matches[match_count++] = node->name;
                }
                node = node->next;
            }
        }
    }

    if (match_count == 1) {
        /* Single match: auto-complete */
        const char* match = matches[0];
        size_t match_len = strlen(match);
        size_t append_len = match_len - prefix_len;

        if (*pos + append_len < size - 1) {
            printf("%s", match + prefix_len);
            strcpy(buffer + *pos, match + prefix_len);
            *pos += append_len;
            buffer[*pos] = '\0';
        }
    } else if (match_count > 1) {
        /* Multiple matches: show options */
        printf("\n");
        for (int i = 0; i < match_count; i++) {
            printf("%-16s ", matches[i]);
            if ((i + 1) % 4 == 0) printf("\n");
        }
        printf("\n");
        
        /* Redisplay prompt and current buffer */
        middleware_display_prompt(middleware);
        printf("%s", buffer); 
    }
}

int cli_get_input(Middleware* middleware, char* buffer, size_t size) {
    int pos = 0;
    int c;

    buffer[0] = '\0';

    while (1) {
        c = portable_getch();

        if (c == KEY_ENTER || c == '\n' || c == '\r') {
            buffer[pos] = '\0';
            printf("\n");
            return 0;
        } else if (c == KEY_BACKSPACE || c == 127) { /* 127 is DEL/Backspace in some terminals */
            if (pos > 0) {
                pos--;
                printf("\b \b");
            }
        } else if (c == KEY_TAB) {
            handle_tab_completion(middleware, buffer, &pos, size);
        } else if (c == KEY_ESC) {
            buffer[0] = '\0';
            printf("\n");
            return -1;
        } else if (isprint(c)) {
            if (pos < (int)size - 1) {
                buffer[pos++] = (char)c;
                printf("%c", c);
            }
        }
    }

    return 0;
}
