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

/*********************************************************************
 * Define
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

#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_H */

/*********************************************************************
 * UUID: 7be660d6-55fa-417e-b30d-c44eecf89b71
 *********************************************************************/

