#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdint.h>
#include <stdbool.h>

// Cấu trúc node trong danh sách liên kết
typedef struct ListNode {
    void* data;                  // Con trỏ đến dữ liệu
    struct ListNode* next;       // Con trỏ đến node tiếp theo
} ListNode;

// Cấu trúc danh sách liên kết
typedef struct {
    ListNode* head;              // Con trỏ đến node đầu tiên
    ListNode* tail;              // Con trỏ đến node cuối cùng
    uint32_t size;               // Số lượng node trong danh sách
    void (*free_data)(void*);    // Hàm giải phóng dữ liệu
} LinkedList;

/**
 * Khởi tạo danh sách liên kết
 * @param list Con trỏ đến cấu trúc LinkedList
 * @param free_data Hàm giải phóng dữ liệu (có thể NULL)
 * @return true nếu thành công, false nếu thất bại
 */
bool linkedlist_init(LinkedList* list, void (*free_data)(void*));

/**
 * Thêm node vào cuối danh sách
 * @param list Con trỏ đến cấu trúc LinkedList
 * @param data Con trỏ đến dữ liệu
 * @return true nếu thành công, false nếu thất bại
 */
bool linkedlist_append(LinkedList* list, void* data);

/**
 * Thêm node vào đầu danh sách
 * @param list Con trỏ đến cấu trúc LinkedList
 * @param data Con trỏ đến dữ liệu
 * @return true nếu thành công, false nếu thất bại
 */
bool linkedlist_prepend(LinkedList* list, void* data);

/**
 * Lấy node tại vị trí chỉ định
 * @param list Con trỏ đến cấu trúc LinkedList
 * @param index Vị trí cần lấy
 * @return Con trỏ đến node nếu thành công, NULL nếu thất bại
 */
ListNode* linkedlist_get_node(LinkedList* list, uint32_t index);

/**
 * Lấy dữ liệu tại vị trí chỉ định
 * @param list Con trỏ đến cấu trúc LinkedList
 * @param index Vị trí cần lấy
 * @return Con trỏ đến dữ liệu nếu thành công, NULL nếu thất bại
 */
void* linkedlist_get(LinkedList* list, uint32_t index);

/**
 * Xóa node tại vị trí chỉ định
 * @param list Con trỏ đến cấu trúc LinkedList
 * @param index Vị trí cần xóa
 * @return true nếu thành công, false nếu thất bại
 */
bool linkedlist_remove(LinkedList* list, uint32_t index);

/**
 * Xóa toàn bộ danh sách
 * @param list Con trỏ đến cấu trúc LinkedList
 */
void linkedlist_clear(LinkedList* list);

/**
 * Lấy kích thước danh sách
 * @param list Con trỏ đến cấu trúc LinkedList
 * @return Số lượng node trong danh sách
 */
uint32_t linkedlist_size(LinkedList* list);

/**
 * Kiểm tra danh sách có rỗng không
 * @param list Con trỏ đến cấu trúc LinkedList
 * @return true nếu danh sách rỗng, false nếu không
 */
bool linkedlist_is_empty(LinkedList* list);

/**
 * Duyệt danh sách và áp dụng hàm callback cho mỗi node
 * @param list Con trỏ đến cấu trúc LinkedList
 * @param callback Hàm callback
 * @param user_data Dữ liệu người dùng truyền vào callback
 */
void linkedlist_foreach(LinkedList* list, void (*callback)(void* data, void* user_data), void* user_data);

#endif // LINKEDLIST_H
