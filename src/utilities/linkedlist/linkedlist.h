/*
* Linked List Header
* Author: Ducson9112k
*
* Description:
*   Định nghĩa các hàm và kiểu dữ liệu cho danh sách liên kết.
*/

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../../common/common_types.h"

/*------------------------------*
*    Status Codes
*------------------------------*/
#define LIST_SUCCESS 0
#define LIST_ERROR -1
#define LIST_INVALID_PARAMETER -2

/*------------------------------*
*    Linked List Data Types
*------------------------------*/

/* Node của danh sách liên kết */
typedef struct linkedlist_node {
    void *data;           /* Con trỏ đến dữ liệu */
    struct linkedlist_node *next;    /* Con trỏ đến node tiếp theo */
} linkedlist_node_t;

/* Danh sách liên kết */
typedef struct {
    linkedlist_node_t *head;         /* Con trỏ đến node đầu */
    linkedlist_node_t *tail;         /* Con trỏ đến node cuối */
    uint32_t size;        /* Kích thước danh sách */
} linkedlist_t;

/* Compare Function Type */
typedef int32_t (*compare_fn)(const void *a, const void *b);

/* Free Function Type */
typedef void (*free_fn)(void *data);

/*------------------------------*
*    Function Prototypes
*------------------------------*/

/**
 * @brief Khởi tạo danh sách liên kết
 *
 * @param list Con trỏ đến danh sách
 */
void linkedlist_init(linkedlist_t *list);

/**
 * @brief Thêm node vào cuối danh sách
 *
 * @param list Con trỏ đến danh sách
 * @param data Con trỏ đến dữ liệu
 * @return LIST_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t linkedlist_append(linkedlist_t *list, void *data);

/**
 * @brief Thêm node vào đầu danh sách
 *
 * @param list Con trỏ đến danh sách
 * @param data Con trỏ đến dữ liệu
 * @return LIST_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t linkedlist_prepend(linkedlist_t *list, void *data);

/**
 * @brief Xóa node đầu tiên
 *
 * @param list Con trỏ đến danh sách
 * @return Con trỏ đến dữ liệu đã xóa, NULL nếu danh sách rỗng
 */
void *linkedlist_remove_first(linkedlist_t *list);

/**
 * @brief Xóa node cuối cùng
 *
 * @param list Con trỏ đến danh sách
 * @return Con trỏ đến dữ liệu đã xóa, NULL nếu danh sách rỗng
 */
void *linkedlist_remove_last(linkedlist_t *list);

/**
 * @brief Lấy kích thước danh sách
 *
 * @param list Con trỏ đến danh sách
 * @return Kích thước danh sách
 */
uint32_t linkedlist_size(const linkedlist_t *list);

/**
 * @brief Kiểm tra danh sách rỗng
 *
 * @param list Con trỏ đến danh sách
 * @return true nếu rỗng, false nếu không rỗng
 */
bool linkedlist_is_empty(const linkedlist_t *list);

/**
 * @brief Giải phóng danh sách
 *
 * @param list Con trỏ đến danh sách
 */
void linkedlist_cleanup(linkedlist_t *list);

/**
 * @brief Tìm node trong danh sách
 *
 * @param list Con trỏ đến danh sách
 * @param data Con trỏ đến dữ liệu cần tìm
 * @param compare Hàm so sánh dữ liệu
 * @return Con trỏ đến node tìm thấy, NULL nếu không tìm thấy
 */
linkedlist_node_t *linkedlist_find(const linkedlist_t *list, const void *data, compare_fn compare);

/**
 * @brief Xóa node theo dữ liệu
 *
 * @param list Con trỏ đến danh sách
 * @param data Con trỏ đến dữ liệu cần xóa
 * @param compare Hàm so sánh dữ liệu
 * @return Con trỏ đến dữ liệu đã xóa, NULL nếu không tìm thấy
 */
void *linkedlist_remove(linkedlist_t *list, const void *data, compare_fn compare);

/**
 * @brief Xóa tất cả các node
 *
 * @param list Con trỏ đến danh sách
 * @param free_data Hàm giải phóng dữ liệu
 */
void linkedlist_clear(linkedlist_t *list, free_fn free_data);

#ifdef __cplusplus
}
#endif

#endif /* LINKEDLIST_H */
