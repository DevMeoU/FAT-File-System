/**
 * @file application.h
 * @author Le Duc Son
 * @date 2022-04-20
 * @brief Header file for Application
 */

#ifndef APPLICATION_H
#define APPLICATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../utilities/log/print_color.h"
#include "../common/common_types.h"
#include "../middleware/middleware.h"

/**
 * @brief Application structure
 */
typedef struct {
    Middleware* middleware;
    char env_path[500];
    bool running;
} Application;

/**
 * @brief Initialize Application
 * @param app Pointer to Application structure
 * @return 0 if success, -1 if fail
 */
int application_init(Application* app, Middleware* middleware);

/**
 * @brief Deinitialize Application
 * @param app Pointer to Application structure
 * @return 0 if success, -1 if fail
 */
int application_denit(Application* app);

/**
 * @brief Display prompt
 * @param app Pointer to Application structure
 */
void display_prompt(Application* app);

/**
 * @brief Run Application
 * @param app Pointer to Application structure
 * @param img_path Path to image file
 * @param mode Mode (read-only or read-write)
 * @return 0 if success, -1 if fail
 */
int application_run(Application* app);

/**
 * @brief Process command from user
 * @param app Pointer to Application structure
 * @param command Command to process
 * @return 0 if success, -1 if fail
 */
char* trim(char* str);

/**
 * @brief Custom strtok_r()
 * @param str String to split
 * @param delim Delimiter
 * @param saveptr Pointer to save position
 * @return Split string
 */
char *custom_strtok_r(char *str, const char *delim, char **saveptr);

/**
 * @brief Process command from user
 * @param app Pointer to Application structure
 * @param command Command to process
 * @return 0 if success, -1 if fail
 */
int process_command_with_and(Application* app, const char* command);

/**
 * @brief Process command from user
 * @param app Pointer to Application structure
 * @param command Command to process
 * @return 0 if success, -1 if fail
 */
int application_process_command(Application* app, const char* command);

/**
 * @brief Show help
 * @param app Pointer to Application structure
 */
void application_show_help(Application* app);

/**
 * @brief Stop Application
 * @param app Pointer to Application structure
 */
void application_stop(Application* app);

#endif // APPLICATION_H

