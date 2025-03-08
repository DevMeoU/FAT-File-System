/*
* Linked List Header
* Author: Ducson9112k
*
* Description:
*   Định nghĩa các hàm và kiểu dữ liệu cho danh sách liên kết.
*/

#ifndef __LINKEDLIST_H
#define __LINKEDLIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../../common/common_types.h"

/*------------------------------*
*    Linked List Data Types
*------------------------------*/

/* Node của danh sách liên kết */
typedef struct node {
    void *data;           /* Con trỏ đến dữ liệu */
    struct node *next;    /* Con trỏ đến node tiếp theo */
    struct node *prev;    /* Con trỏ đến node trước */
} node_t;

/* Danh sách liên kết */
typedef struct {
    node_t *head;         /* Con trỏ đến node đầu */
    node_t *tail;         /* Con trỏ đến node cuối */
    uint32_t size;        /* Kích thước danh sách */
    bool is_circular;     /* Danh sách vòng */
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
 * @param is_circular Danh sách vòng hay không
 * @return LIST_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t list_init(linkedlist_t *list, bool is_circular);

/**
 * @brief Thêm node vào đầu danh sách
 *
 * @param list Con trỏ đến danh sách
 * @param data Con trỏ đến dữ liệu
 * @return LIST_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t list_push_front(linkedlist_t *list, void *data);

/**
 * @brief Thêm node vào cuối danh sách
 *
 * @param list Con trỏ đến danh sách
 * @param data Con trỏ đến dữ liệu
 * @return LIST_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t list_push_back(linkedlist_t *list, void *data);

/**
 * @brief Xóa node đầu danh sách
 *
 * @param list Con trỏ đến danh sách
 * @param free_func Hàm giải phóng dữ liệu
 * @return LIST_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t list_pop_front(linkedlist_t *list, free_fn free_func);

/**
 * @brief Xóa node cuối danh sách
 *
 * @param list Con trỏ đến danh sách
 * @param free_func Hàm giải phóng dữ liệu
 * @return LIST_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t list_pop_back(linkedlist_t *list, free_fn free_func);

/**
 * @brief Chèn node vào vị trí chỉ định
 *
 * @param list Con trỏ đến danh sách
 * @param data Con trỏ đến dữ liệu
 * @param index Vị trí chèn
 * @return LIST_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t list_insert(linkedlist_t *list, void *data, uint32_t index);

/**
 * @brief Xóa node tại vị trí chỉ định
 *
 * @param list Con trỏ đến danh sách
 * @param index Vị trí xóa
 * @param free_func Hàm giải phóng dữ liệu
 * @return LIST_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t list_remove(linkedlist_t *list, uint32_t index, free_fn free_func);

/**
 * @brief Tìm kiếm node trong danh sách
 *
 * @param list Con trỏ đến danh sách
 * @param data Con trỏ đến dữ liệu cần tìm
 * @param compare_func Hàm so sánh
 * @return Con trỏ đến node nếu tìm thấy, NULL nếu không tìm thấy
 */
node_t *list_find(linkedlist_t *list, const void *data, compare_fn compare_func);

/**
 * @brief Xóa toàn bộ danh sách
 *
 * @param list Con trỏ đến danh sách
 * @param free_func Hàm giải phóng dữ liệu
 * @return LIST_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t list_clear(linkedlist_t *list, free_fn free_func);

/**
 * @brief Lấy kích thước danh sách
 *
 * @param list Con trỏ đến danh sách
 * @return Kích thước danh sách
 */
uint32_t list_size(const linkedlist_t *list);

/**
 * @brief Kiểm tra danh sách rỗng
 *
 * @param list Con trỏ đến danh sách
 * @return true nếu rỗng, false nếu không rỗng
 */
bool list_is_empty(const linkedlist_t *list);

#ifdef __cplusplus
}
#endif

#endif /* __LINKEDLIST_H */
