/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Module linkedlist cung cấp cấu trúc dữ liệu danh sách liên kết
 *   và các hàm để thao tác với danh sách.
 *********************************************************************/
#ifndef __MODULE_LINKEDLIST_H
#define __MODULE_LINKEDLIST_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/*********************************************************************
 * Macro Definitions
 *********************************************************************/

/* Error codes */
#define LIST_SUCCESS         0
#define LIST_ERROR         -1
#define LIST_EMPTY         -2
#define LIST_NOT_FOUND     -3
#define LIST_DUPLICATE     -4

/*********************************************************************
 * Type Definitions
 *********************************************************************/

/* Node structure */
typedef struct list_node {
    void *data;              /* Con trỏ đến dữ liệu */
    struct list_node *next;  /* Con trỏ đến node tiếp theo */
    struct list_node *prev;  /* Con trỏ đến node trước */
} list_node_t;

/* List structure */
typedef struct {
    list_node_t *head;       /* Con trỏ đến node đầu */
    list_node_t *tail;       /* Con trỏ đến node cuối */
    uint32_t size;           /* Số lượng node trong list */
    bool is_circular;        /* List có phải dạng vòng không */
} list_t;

/* Compare function type */
typedef int32_t (*list_compare_fn)(const void *a, const void *b);

/* Free function type */
typedef void (*list_free_fn)(void *data);

/*********************************************************************
 * Public Function Prototypes
 *********************************************************************/

/**
 * @brief Khởi tạo danh sách
 *
 * @param list Con trỏ đến list cần khởi tạo
 * @param is_circular true nếu muốn tạo list dạng vòng
 * @return LIST_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t list_init(list_t *list, bool is_circular);

/**
 * @brief Thêm node vào đầu danh sách
 *
 * @param list Con trỏ đến list
 * @param data Con trỏ đến dữ liệu cần thêm
 * @return LIST_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t list_push_front(list_t *list, void *data);

/**
 * @brief Thêm node vào cuối danh sách
 *
 * @param list Con trỏ đến list
 * @param data Con trỏ đến dữ liệu cần thêm
 * @return LIST_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t list_push_back(list_t *list, void *data);

/**
 * @brief Xóa node đầu danh sách
 *
 * @param list Con trỏ đến list
 * @param free_fn Hàm giải phóng dữ liệu
 * @return LIST_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t list_pop_front(list_t *list, list_free_fn free_fn);

/**
 * @brief Xóa node cuối danh sách
 *
 * @param list Con trỏ đến list
 * @param free_fn Hàm giải phóng dữ liệu
 * @return LIST_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t list_pop_back(list_t *list, list_free_fn free_fn);

/**
 * @brief Tìm kiếm node trong danh sách
 *
 * @param list Con trỏ đến list
 * @param data Dữ liệu cần tìm
 * @param compare_fn Hàm so sánh dữ liệu
 * @return Con trỏ đến node nếu tìm thấy, NULL nếu không tìm thấy
 */
list_node_t *list_find(const list_t *list, const void *data, 
                       list_compare_fn compare_fn);

/**
 * @brief Xóa toàn bộ danh sách
 *
 * @param list Con trỏ đến list
 * @param free_fn Hàm giải phóng dữ liệu
 */
void list_clear(list_t *list, list_free_fn free_fn);

#ifdef __cplusplus
}
#endif

#endif /* __MODULE_LINKEDLIST_H */

/*********************************************************************
 * UUID: 7be660d6-55fa-417e-b30d-c44eecf89b71
 *********************************************************************/ 