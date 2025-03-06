/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 *********************************************************************/
#ifndef APPLICATION_H
#define APPLICATION_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * Include
 *********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "print_color.h"

/*********************************************************************
 * Define
 *********************************************************************/

/*********************************************************************
 * Static function
 *********************************************************************/

/*********************************************************************
 * Function prototypes
 *********************************************************************/

/**
 * @brief Initialize the application.
 * 
 * @return 0 if the initialization is successful, -1 otherwise.
 */
int app_init(void);

/**
 * @brief Run the application.
 * 
 * This function will run the main loop of the application.
 */
void app_run(void);

/**
 * @brief Exit the application.
 * 
 * This function will clean up and exit the application.
 */
void app_exit(void);

/**
 * @brief Handle the given command.
 * 
 * This function will parse the given command and call the appropriate
 * middleware function.
 * 
 * @param command The command to handle.
 */
void app_handle_command(char *command);

/**
 * @brief Print the application title.
 * 
 * @param current_path The current directory path.
 */
void app_print_title(char *current_path);

/**
 * @brief Print the help message.
 */
void app_print_help(void);

/**
 * @brief Clears the terminal screen.
 * 
 * This function sends the appropriate command to the terminal to clear
 * the screen, providing a clean interface for the user.
 */
void app_clear_screen();

#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_H */

/*********************************************************************
 * UUID: 7be660d6-55fa-417e-b30d-c44eecf89b71
 *********************************************************************/

