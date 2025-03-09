/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   File định nghĩa các mã lỗi cho FAT File System module.
 *********************************************************************/
#ifndef FAT_DRIVER_ERRORS_H
#define FAT_DRIVER_ERRORS_H

#include <stdint.h>

/* Error codes */
#define FAT_ERROR_SUCCESS             0
#define FAT_ERROR_INVALID_PARAMETER   -1
#define FAT_ERROR_NOT_INITIALIZED     -2
#define FAT_ERROR_ALREADY_INITIALIZED -3
#define FAT_ERROR_INVALID_BOOT_SECTOR -4
#define FAT_ERROR_INVALID_SECTOR_SIZE -5
#define FAT_ERROR_READ_FAILED         -6
#define FAT_ERROR_WRITE_FAILED        -7
#define FAT_ERROR_FILE_NOT_FOUND      -8
#define FAT_ERROR_FILE_EXISTS         -9
#define FAT_ERROR_DIR_NOT_FOUND       -10
#define FAT_ERROR_DIR_EXISTS          -11
#define FAT_ERROR_NO_SPACE            -12
#define FAT_ERROR_ACCESS_DENIED       -13
#define FAT_ERROR_INVALID_PATH        -14
#define FAT_ERROR_INVALID_NAME        -15
#define FAT_ERROR_INVALID_OPERATION   -16

#endif /* FAT_DRIVER_ERRORS_H */

/*********************************************************************
 * UUID: 4b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/ 