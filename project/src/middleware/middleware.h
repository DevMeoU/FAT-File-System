#ifndef MIDDLEWARE_H
#define MIDDLEWARE_H

#include "../common/common_types.h"
#include "../fat_driver/fat_driver.h"
#include "../fat_driver/fat_driver_types.h"

/**
 * @file middleware.h
 * @brief Declaration of the Middleware structure and functions
 * @date 2023-10-20
 * @author Le Duc Son
 */

#ifndef PATH_MAX
#define PATH_MAX 256 /**< Maximum path length */
#endif

typedef struct {
    const char* img_path;
    FileSystemMode mode;
    FATDriver* fat_driver;
    FileNode* current_directory;
    char current_path[PATH_MAX];
    bool is_root_mode;
} Middleware;

/**
 * Initialize Middleware
 * @param middleware Pointer to the Middleware structure
 * @return 0 if successful, -1 if failed
 */
int middleware_init(Middleware* middleware);

/**
 * Deinitialize Middleware
 * @param middleware Pointer to the Middleware structure
 * @return 0 if successful, -1 if failed
 */
int middleware_denit(Middleware* middleware);

/**
 * Process ls command (list directory contents)
 * @param middleware Pointer to the Middleware structure
 * @param path Optional path to list (NULL for current directory)
 * @return 0 if successful, -1 if failed
 */
int middleware_ls(Middleware* middleware, const char* path);

/**
 * Process cd command (change directory)
 * @param middleware Pointer to the Middleware structure
 * @param path Path to change to
 * @return 0 if successful, -1 if failed
 */
int middleware_cd(Middleware* middleware, const char* path);

/**
 * Process cat command (read file contents)
 * @param middleware Pointer to the Middleware structure
 * @param path Path to the file to read
 * @return 0 if successful, -1 if failed
 */
int middleware_cat(Middleware* middleware, const char* path);

/**
 * Process evidence command (display filesystem info)
 * @param middleware Pointer to the Middleware structure
 * @return 0 if successful, -1 if failed
 */
int middleware_evidence(Middleware* middleware);

/**
 * Switch to root mode
 * @param middleware Pointer to the Middleware structure
 * @return 0 if successful, -1 if failed
 */
int middleware_switch_to_root_mode(Middleware* middleware);

/**
 * Switch to user mode
 * @param middleware Pointer to the Middleware structure
 * @return 0 if successful, -1 if failed
 */
int middleware_switch_to_user_mode(Middleware* middleware);

/**
 * Get current path
 * @param middleware Pointer to the Middleware structure
 * @return Current path string
 */
const char* middleware_get_current_path(Middleware* middleware);

/**
 * Check if in root mode
 * @param middleware Pointer to the Middleware structure
 * @return true if in root mode, false otherwise
 */
bool middleware_is_root_mode(Middleware* middleware);

#endif /* MIDDLEWARE_H */


