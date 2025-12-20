/**
 * @file application.c
 * @brief Implementation of the Application structure and functions
 * @date 2022-04-20
 * @author Le Duc Son
 */

#include "application.h"

/**
 * Initialize the Application structure
 * @param app Pointer to the Application structure
 * @param middleware Pointer to the Middleware structure
 * @return 0 if successful, -1 if failed
 */
int application_init(Application* app, Middleware* middleware) {
    if (!app || !middleware) return -1;

    app->middleware = middleware;
    app->running = false;
    if (-1 != middleware_init(middleware)) {
        app->running = true;
        return 0;
    }

    return -1;
}

/**
 * Deinitialize the Application structure
 * @param app Pointer to the Application structure
 * 
 * This function resets the middleware pointer to NULL and sets the
 * running flag to false, effectively cleaning up the application state.
 */

int application_denit(Application* app) {
    if (!app) return -1;

    app->middleware = NULL;
    app->running = false;

    return 0;
}

/**
 * Display the prompt
 * @param app Pointer to the Application structure
 */
void display_prompt(Application* app) {
    /* Display the prompt */
    if (middleware_is_root_mode(app->middleware)) {
        print_color(COLOR_BOLD COLOR_UNDERLINE COLOR_GREEN, "DEESOL");
        print_color(COLOR_BOLD, "@");
        print_color(COLOR_ITALIC COLOR_GREEN, "root: ");
    } else {
        print_color(COLOR_BOLD COLOR_UNDERLINE COLOR_GREEN, "DEESOL");
        print_color(COLOR_BOLD, "@");
        print_color(COLOR_ITALIC COLOR_BLUE, "user: ");
    }
    print_color(COLOR_MAGENTA, "%s", middleware_get_current_path(app->middleware));
    print_color(COLOR_YELLOW, "$> ");
    fflush(stdout);
}

/**
 * Run the application
 * @param app Pointer to the Application structure
 * @return 0 if successful, -1 if failed
 */
int application_run(Application* app) {
    /* Main loop of the application */
    char command[256];

    while (app->running) {
        /* Display the prompt */
        display_prompt(app);

        /* Read the command */
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }

        /* Remove the newline character */
        size_t len = strlen(command);
        if (len > 0 && command[len - 1] == '\n') {
            command[len - 1] = '\0';
        }

        /* Preprocess the command && */
        if (process_command_with_and(app, command) == 0) {
            continue;
        } else {
            /* Process the command */
            if (-1 == application_process_command(app, command)) {
                print_error("Failed to process command\n");
            }
        }

    }

    /* middleware_denit is called in main */

    return 0;
}

/**
 * Function to trim whitespace from the beginning and end of a string
 * @param str String to trim
 * @return Trimmed string
 */
char *trim(char *str) {
    while (isspace((unsigned char)*str)) str++;  /* Remove leading whitespace */
    if (*str == 0) return str;  /* If the string is empty, return it immediately */

    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;  /* Remove trailing whitespace */

    end[1] = '\0';  /* Null-terminate the string */
    return str;
}

/**
 * Custom implementation of strtok_r()
 * @param str String to split
 * @param delim Delimiter
 * @param saveptr Pointer to save position
 * @return Split string
 */
char *custom_strtok_r(char *str, const char *delim, char **saveptr) {
    if (str) {
        *saveptr = str;
    }
    if (!*saveptr) {
        return NULL;
    }

    str = *saveptr;
    char *end = strstr(str, delim);
    if (end) {
        *end = '\0';
        *saveptr = end + strlen(delim);
    } else {
        *saveptr = NULL;
    }

    return str;
}

/**
 * Process the command with &&
 * @param app Pointer to the Application structure
 * @param command Command to process
 * @return 0 if successful, -1 if failed
 */
int process_command_with_and(Application* app, const char *command) {
    char command_copy[1024];
    strcpy(command_copy, command);  /* Make a copy of the command to modify */

    if (!command) return -1;

    if (strstr(command, "&&") == NULL) {
        return -1;
    }

    char *saveptr = NULL;
    char *cmd = custom_strtok_r(command_copy, "&&", &saveptr);

    while (cmd) {
        cmd = trim(cmd);
        if (*cmd != '\0') {
            /* Process the command here */
            display_prompt(app);
            if (application_process_command(app, cmd)) {
                return -1;
            }
        }
        cmd = custom_strtok_r(NULL, "&&", &saveptr);
    }
    return 0;
}

/**
 * Process the command
 * @param app Pointer to the Application structure
 * @param command Command to process
 * @return 0 if successful, -1 if failed
 */
int application_process_command(Application* app, const char* command) {
    if (!app || !command) return -1;

    /* Parse the command and argument */
    char cmd_copy[256];
    strncpy(cmd_copy, command, sizeof(cmd_copy) - 1);
    cmd_copy[sizeof(cmd_copy) - 1] = '\0';

    char* cmd = strtok(cmd_copy, " ");
    if (!cmd) return 0;

    /* Handle the commands */
    if (strcmp(cmd, "ls") == 0) {
        char* path = strtok(NULL, " ");
        return middleware_ls(app->middleware, path);
    } else if (strcmp(cmd, "cd") == 0) {
        char* path = strtok(NULL, " ");
        if (!path) {
            print_error("cd: missing operand\n");
            return -1;
        }
        return middleware_cd(app->middleware, path);
    } else if (strcmp(cmd, "cat") == 0) {
        char* path = strtok(NULL, " ");
        if (!path) {
            print_error("cat: missing operand\n");
            return -1;
        }
        return middleware_cat(app->middleware, path);
    } else if (strcmp(cmd, "evidence") == 0) {
        return middleware_evidence(app->middleware);
    } else if (strcmp(cmd, "cls") == 0 || strcmp(cmd, "clear") == 0) {
        int ret = system("clear");
        (void)ret; /* Avoid unused variable warning */
        return 0;
    } else if (strcmp(cmd, "help") == 0) {
        application_show_help(app);
        return 0;
    } else if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) {
        application_stop(app);
        return 0;
    } else {
        print_error("Unknown command: %s\n", cmd);
        print_info("Type 'help' for available commands\n");
        return -1;
    }
}

/**
 * Show the help message
 * @param app Pointer to the Application structure
 */
void application_show_help(Application* app) {
    (void)app; /* Avoid unused parameter warning */

    printf("Available commands:\n");
    printf("  ls [path]                 List files and directories\n");
    printf("  cd <path>                 Change directory\n");
    printf("  cat <file>                Display file content\n");
    printf("  evidence                  Show file system information\n");
    printf("  cls, clear                Clear the screen\n");
    printf("  help                      Show this help message\n");
    printf("  exit, quit                Exit the program\n");
}

/**
 * Stop the application
 * @param app Pointer to the Application structure
 */
void application_stop(Application* app) {
    if (!app) return;

    app->running = false;
    print_info("Exiting...\n");
}

/**
 * Main function
 * @param argc Argument count
 * @param argv Argument vector
 * @return 0 if successful, 1 if failed
 */
int main(int argc, char* argv[]) {
    /* Check the command line arguments */
    if (argc < 2) {
        print_warning("Usage: %s <img_file> [mode]\n", argv[0]);
        print_info("  <img_file>: Path to the image file\n");
        print_info("  [mode]: Optional, 'read-only' (default) or 'read-write'\n");
        return 1;
    }

    const char* img_path = argv[1];
    FileSystemMode mode = MODE_READ_ONLY; /* Default to read-only mode */

    /* Handle the mode argument if it exists */
    if (argc >= 3) {
        if (strcmp(argv[2], "read-write") == 0) {
            mode = MODE_READ_WRITE;
        } else if (strcmp(argv[2], "read-only") == 0) {
            mode = MODE_READ_ONLY;
        } else {
            print_error("Invalid mode: %s\n", argv[2]);
            print_info("Mode must be 'read-only' or 'read-write'\n");
            return 1;
        }
    }

    /* Initialize and run the application */
    Application app;
    Middleware middleware = {
        .img_path = img_path,
        .mode = mode,
        .fat_driver = NULL,
        .current_directory = NULL,
        .current_path = "/", /* Current directory is root */
        .is_root_mode = false
    };

    if (application_init(&app, &middleware) != 0) {
        print_error("Failed to initialize application\n");
        return 1;
    }

    int result = application_run(&app);

    /* Free resources */
    middleware_denit(&middleware);
    application_denit(&app);

    /* Return error code if application_run failed */
    return result == 0 ? 0 : 1;
}
