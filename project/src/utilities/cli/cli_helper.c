#include "cli_helper.h"
#include "../../middleware/middleware.h"
#include "../../utilities/log/print_color.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#define strncasecmp _strnicmp
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <strings.h>
#endif

#define MAX_HISTORY 20
#define MAX_BUFFER_SIZE 256

static char s_history[MAX_HISTORY][MAX_BUFFER_SIZE];
static int s_history_count = 0;
static int s_history_index = -1;
static char s_current_buffer_backup[MAX_BUFFER_SIZE] = {0};

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
                /* Handle special keys (arrows) */
                int vk = ir.Event.KeyEvent.wVirtualKeyCode;
                if (vk == VK_UP) return KEY_UP;
                if (vk == VK_DOWN) return KEY_DOWN;
                if (vk == VK_LEFT) return KEY_LEFT;
                if (vk == VK_RIGHT) return KEY_RIGHT;
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
    if (ch == 27) { /* Escape sequence? */
        /* Set non-blocking to check for more chars */
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
        
        int n2 = getchar();
        if (n2 == '[') {
            int n3 = getchar();
            if (n3 == 'A') ch = KEY_UP;
            else if (n3 == 'B') ch = KEY_DOWN;
            else if (n3 == 'C') ch = KEY_RIGHT;
            else if (n3 == 'D') ch = KEY_LEFT;
        } else {
            /* Just a single ESC key or unknown sequence */
            /* Put back n2 if it wasn't EOF? (ungetc doesn't work well with ESC) */
        }
        
        fcntl(STDIN_FILENO, F_SETFL, flags);
    }
    
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
    char* prefix_to_match = (last_space == NULL) ? completion_prefix : (last_space + 1);
    int is_command = (last_space == NULL);

    size_t initial_prefix_len = strlen(prefix_to_match);
    if (initial_prefix_len == 0 && !is_command) return;

    const char* matches[64];
    int match_count = 0;

    if (is_command) {
        for (int i = 0; s_commands[i] != NULL; i++) {
            if (strncmp(prefix_to_match, s_commands[i], initial_prefix_len) == 0) {
                matches[match_count++] = s_commands[i];
            }
        }
    } else {
        FileNode* search_dir = middleware->current_directory;
        char* last_slash = strrchr(prefix_to_match, '/');
        const char* name_prefix = prefix_to_match;

        if (last_slash != NULL) {
            char dir_path[256];
            size_t dir_len = (size_t)(last_slash - prefix_to_match);
            if (dir_len == 0) {
                search_dir = middleware->fat_driver->root_directory;
            } else if (dir_len < sizeof(dir_path)) {
                strncpy(dir_path, prefix_to_match, dir_len);
                dir_path[dir_len] = '\0';
                search_dir = fat_driver_find_path(middleware->fat_driver, dir_path);
            }
            name_prefix = last_slash + 1;
        }

        if (search_dir && search_dir->type == FILE_TYPE_DIRECTORY) {
            fat_driver_list_directory(middleware->fat_driver, search_dir);
            size_t n_prefix_len = strlen(name_prefix);
            FileNode* node = search_dir->children;
            while (node && match_count < 64) {
                if (strncasecmp(name_prefix, node->name, n_prefix_len) == 0) {
                    matches[match_count++] = node->name;
                }
                node = node->next;
            }
            initial_prefix_len = n_prefix_len;
        }
    }

    if (match_count == 1) {
        const char* s_match = matches[0];
        size_t m_len = strlen(s_match);
        size_t a_len = m_len - initial_prefix_len;

        if (*pos + a_len < size - 1) {
            printf("%s", s_match + initial_prefix_len);
            strcpy(buffer + *pos, s_match + initial_prefix_len);
            *pos += (int)a_len;

            if (!is_command) {
                FileNode* s_dir = middleware->current_directory;
                char* l_slash = strrchr(prefix_to_match, '/');
                if (l_slash) {
                    char d_path[256];
                    size_t d_len = (size_t)(l_slash - prefix_to_match);
                    if (d_len == 0) s_dir = middleware->fat_driver->root_directory;
                    else if (d_len < sizeof(d_path)) {
                        strncpy(d_path, prefix_to_match, d_len);
                        d_path[d_len] = '\0';
                        s_dir = fat_driver_find_path(middleware->fat_driver, d_path);
                    }
                }
                if (s_dir) {
                    FileNode* n = s_dir->children;
                    while (n) {
                        if (strcmp(n->name, s_match) == 0) {
                            if (n->type == FILE_TYPE_DIRECTORY && *pos < (int)size - 1) {
                                printf("/");
                                buffer[(*pos)++] = '/';
                            }
                            break;
                        }
                        n = n->next;
                    }
                }
            }
            buffer[*pos] = '\0';
        }
    } else if (match_count > 1) {
        printf("\n");
        for (int i = 0; i < match_count; i++) {
            printf("%-16s ", matches[i]);
            if ((i + 1) % 4 == 0) printf("\n");
        }
        printf("\n");
        middleware_display_prompt(middleware);
        printf("%s", buffer); 
    }
}

static void add_to_history(const char* command) {
    if (command == NULL || strlen(command) == 0) return;
    
    /* Don't add if same as last command */
    if (s_history_count > 0) {
        if (strcmp(s_history[(s_history_count - 1) % MAX_HISTORY], command) == 0) return;
    }
    
    strncpy(s_history[s_history_count % MAX_HISTORY], command, MAX_BUFFER_SIZE - 1);
    s_history[s_history_count % MAX_HISTORY][MAX_BUFFER_SIZE - 1] = '\0';
    s_history_count++;
}

int cli_get_input(Middleware* middleware, char* buffer, size_t size) {
    int pos = 0;        /* Number of characters in buffer */
    int cursor_pos = 0; /* Current cursor position (0 to pos) */
    int c;
    buffer[0] = '\0';
    s_history_index = s_history_count;

    while (1) {
        c = portable_getch();
        if (c == KEY_ENTER || c == '\n' || c == '\r') {
            buffer[pos] = '\0';
            printf("\n");
            add_to_history(buffer);
            return 0;
        } else if (c == KEY_BACKSPACE || c == 127) {
            if (cursor_pos > 0) {
                /* Shift buffer left */
                memmove(buffer + cursor_pos - 1, buffer + cursor_pos, pos - cursor_pos + 1);
                pos--;
                cursor_pos--;
                
                /* Visual update: Move back, print tail, clear last char, move back to cursor */
                printf("\b%s ", buffer + cursor_pos);
                for (int i = 0; i < pos - cursor_pos + 1; i++) printf("\b");
            }
        } else if (c == KEY_LEFT) {
            if (cursor_pos > 0) {
                cursor_pos--;
                printf("\033[D"); /* Move cursor left */
            }
        } else if (c == KEY_RIGHT) {
            if (cursor_pos < pos) {
                cursor_pos++;
                printf("\033[C"); /* Move cursor right */
            }
        } else if (c == KEY_TAB) {
            handle_tab_completion(middleware, buffer, &pos, size);
            cursor_pos = pos; /* Completion always happens at end for now */
        } else if (c == KEY_ESC) {
            buffer[0] = '\0';
            pos = 0;
            cursor_pos = 0;
            printf("\n");
            middleware_display_prompt(middleware);
        } else if (c == KEY_UP) {
            if (s_history_count > 0 && s_history_index > s_history_count - MAX_HISTORY && s_history_index > 0) {
                if (s_history_index == s_history_count) {
                    strncpy(s_current_buffer_backup, buffer, MAX_BUFFER_SIZE - 1);
                }
                s_history_index--;
                
                /* Clear current line visually */
                while (cursor_pos < pos) { printf("\033[C"); cursor_pos++; }
                while (pos > 0) { printf("\b \b"); pos--; }
                
                const char* hist = s_history[s_history_index % MAX_HISTORY];
                strncpy(buffer, hist, size - 1);
                buffer[size - 1] = '\0';
                pos = (int)strlen(buffer);
                cursor_pos = pos;
                printf("%s", buffer);
            }
        } else if (c == KEY_DOWN) {
            if (s_history_index < s_history_count) {
                s_history_index++;
                
                /* Clear current line visually */
                while (cursor_pos < pos) { printf("\033[C"); cursor_pos++; }
                while (pos > 0) { printf("\b \b"); pos--; }
                
                if (s_history_index == s_history_count) {
                    strncpy(buffer, s_current_buffer_backup, size - 1);
                } else {
                    const char* hist = s_history[s_history_index % MAX_HISTORY];
                    strncpy(buffer, hist, size - 1);
                }
                buffer[size - 1] = '\0';
                pos = (int)strlen(buffer);
                cursor_pos = pos;
                printf("%s", buffer);
            }
        } else if (c >= 32 && c <= 126) {
            if (pos < (int)size - 1) {
                /* Shift tail right to make space for insertion */
                memmove(buffer + cursor_pos + 1, buffer + cursor_pos, pos - cursor_pos + 1);
                buffer[cursor_pos] = (char)c;
                pos++;
                
                /* Visual update: print character and everything after it */
                printf("%s", buffer + cursor_pos);
                cursor_pos++;
                
                /* Move cursor back to the right position */
                for (int i = 0; i < pos - cursor_pos; i++) printf("\b");
            }
        }
    }
}
