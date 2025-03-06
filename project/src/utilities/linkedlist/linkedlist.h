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

#include <stddef.h>

/*------------------------------*
*    Linked List Data Types
*------------------------------*/

/* Node của danh sách liên kết */
typedef struct llist_node {
    void *data;                 /* Con trỏ đến dữ liệu */
    size_t data_size;           /* Kích thước dữ liệu (byte) */
    struct llist_node *next;    /* Con trỏ đến node kế tiếp */
} llist_node_t;

/* Danh sách liên kết */
typedef struct linkedlist {
    llist_node_t *head;         /* Node đầu danh sách */
    llist_node_t *tail;         /* Node cuối danh sách */
    size_t count;               /* Số lượng node */
} linkedlist_t;

/* Iterator cho danh sách liên kết */
typedef struct linkedlist_iterator {
    llist_node_t *current;
} linkedlist_iterator_t;

/*------------------------------*
*    Function Prototypes
*------------------------------*/

/**
 * @brief Khởi tạo một danh sách liên kết rỗng.
 *
 * @return Con trỏ đến danh sách liên kết hoặc NULL nếu thất bại.
 */
linkedlist_t *llist_init(void);

/**
 * @brief Thêm một node chứa dữ liệu vào cuối danh sách.
 *
 * @param list Con trỏ đến danh sách.
 * @param data Con trỏ đến dữ liệu cần thêm.
 * @param data_size Kích thước của dữ liệu (byte).
 * @return 0 nếu thành công, -1 nếu thất bại.
 */
int llist_add(linkedlist_t *list, const void *data, size_t data_size);

/**
 * @brief Xóa node tại vị trí chỉ định khỏi danh sách.
 *
 * @param list Con trỏ đến danh sách.
 * @param index Vị trí (0-based) của node cần xóa.
 * @return 0 nếu thành công, -1 nếu thất bại.
 */
int llist_remove(linkedlist_t *list, size_t index);

/**
 * @brief Xóa toàn bộ các node trong danh sách (giữ nguyên cấu trúc danh sách).
 *
 * @param list Con trỏ đến danh sách.
 */
void llist_clear(linkedlist_t *list);

/**
 * @brief Hủy và giải phóng toàn bộ bộ nhớ của danh sách.
 *
 * @param list Con trỏ đến danh sách.
 */
void llist_destroy(linkedlist_t *list);

/**
 * @brief Lấy dữ liệu của node đầu tiên trong danh sách.
 *
 * @param list Con trỏ đến danh sách.
 * @return Con trỏ đến dữ liệu của node đầu tiên hoặc NULL nếu danh sách rỗng.
 */
void *llist_get_first(linkedlist_t *list);

/**
 * @brief Khởi tạo iterator cho danh sách.
 *
 * @param list Con trỏ đến danh sách.
 * @param it Con trỏ đến cấu trúc iterator.
 */
void llist_iterator_init(linkedlist_t *list, linkedlist_iterator_t *it);

/**
 * @brief Kiểm tra xem còn node nào trong iterator hay không.
 *
 * @param it Con trỏ đến iterator.
 * @return 1 nếu còn node, 0 nếu không.
 */
int llist_iterator_has_next(linkedlist_iterator_t *it);

/**
 * @brief Lấy dữ liệu của node tiếp theo từ iterator.
 *
 * @param it Con trỏ đến iterator.
 * @return Con trỏ đến dữ liệu của node tiếp theo hoặc NULL nếu không còn.
 */
void *llist_iterator_next(linkedlist_iterator_t *it);

#ifdef __cplusplus
}
#endif

#endif /* LINKEDLIST_H */
