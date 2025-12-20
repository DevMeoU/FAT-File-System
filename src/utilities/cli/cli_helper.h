/**
 * @file cli_helper.h
 * @brief CLI helper utility for command and file suggestions
 */

#ifndef CLI_HELPER_H
#define CLI_HELPER_H

#include <stddef.h>
#include "../../middleware/middleware.h"

#define KEY_TAB       9
#define KEY_ENTER     13
#define KEY_BACKSPACE 8
#define KEY_ESC       27

/**
 * Get input from the CLI with tab-completion support
 * @param middleware Pointer to the Middleware structure for file suggestions
 * @param buffer Buffer to store the input string
 * @param size Size of the buffer
 * @return 0 if successful, -1 if failed or interrupted
 */
int cli_get_input(Middleware* middleware, char* buffer, size_t size);

#endif // CLI_HELPER_H
