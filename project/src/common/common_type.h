/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   File định nghĩa các kiểu dữ liệu và mã lỗi chung cho toàn bộ project.
 *********************************************************************/
#ifndef __COMMON_TYPE_H
#define __COMMON_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * Status Codes
 *********************************************************************/
#define STATUS_SUCCESS          0   /* Thành công */
#define STATUS_ERROR          -1   /* Lỗi chung */
#define STATUS_INVALID        -2   /* Tham số không hợp lệ */
#define STATUS_NOT_FOUND      -3   /* Không tìm thấy */
#define STATUS_EXISTS         -4   /* Đã tồn tại */
#define STATUS_DISK_FULL      -5   /* Đĩa đầy */
#define STATUS_READ_ONLY      -6   /* Chỉ đọc */
#define STATUS_EOF            -7   /* Hết tập tin */
#define STATUS_INVALID_NAME   -8   /* Tên không hợp lệ */
#define STATUS_ROOT_FULL      -9   /* Thư mục gốc đầy */
#define STATUS_INVALID_PATH   -10  /* Đường dẫn không hợp lệ */

#ifdef __cplusplus
}
#endif

#endif /* __COMMON_TYPE_H */

/*********************************************************************
 * UUID: 3b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/ 