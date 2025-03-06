/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 *********************************************************************/

 #ifndef MIDDLEWARE_H
 #define MIDDLEWARE_H
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 /*********************************************************************
  * Include
  *********************************************************************/
 #include <stdio.h>
 #include <string.h>
 
 /*********************************************************************
  * Define
  *********************************************************************/
 /* Hiện tại không có macro nào được định nghĩa trong middleware.c, 
  có thể thêm các macro nếu cần trong tương lai */
 
 /*********************************************************************
  * Function prototypes
  *********************************************************************/
 
 /**
  * @brief Initialize middleware and FAT driver
  * @param img_path Path to the floppy image file
  * @return 0 on success, -1 on failure
  */
 int middleware_init(const char *img_path);
 
 /**
  * @brief List directory contents
  * @param path Path of the directory to list
  */
 void middleware_list_directory(const char *path);
 
 /**
  * @brief Change current directory
  * @param path New directory path
  * @param current_path Current path to update
  */
 void middleware_change_directory(const char *path, char *current_path);
 
 /**
  * @brief Read and display file content
  * @param filename Path of the file to read
  */
 void middleware_read_file(const char *filename);
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /* MIDDLEWARE_H */