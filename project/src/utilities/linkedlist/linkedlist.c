/*
* Linked List Module
* Author: Ducson9112k
*
* Description:
*   Cài đặt các hàm quản lý danh sách liên kết.
*/

#include "linkedlist.h"
#include <stdlib.h>
#include <string.h>

/*--------------------------------------------------------------------
* Linked List Public API Functions
*--------------------------------------------------------------------*/

/*
* llist_init:
*   Cấp phát và khởi tạo một danh sách liên kết rỗng.
*/
linkedlist_t *llist_init(void) {
    linkedlist_t *list = (linkedlist_t *)malloc(sizeof(linkedlist_t));
    if (!list) {
        return NULL;
    }
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
    return list;
}

/*
* llist_add:
*   Thêm một node chứa dữ liệu vào cuối danh sách.
*/
int llist_add(linkedlist_t *list, const void *data, size_t data_size) {
    if (!list || !data || data_size == 0) {
        return -1;
    }
    llist_node_t *new_node = (llist_node_t *)malloc(sizeof(llist_node_t));
    if (!new_node) {
        return -1;
    }
    new_node->data = malloc(data_size);
    if (!new_node->data) {
        free(new_node);
        return -1;
    }
    memcpy(new_node->data, data, data_size);
    new_node->data_size = data_size;
    new_node->next = NULL;
    if (list->tail) {
        list->tail->next = new_node;
        list->tail = new_node;
    } else {
        list->head = new_node;
        list->tail = new_node;
    }
    list->count++;
    return 0;
}

/*
* llist_remove:
*   Xóa node tại vị trí chỉ định khỏi danh sách.
*/
int llist_remove(linkedlist_t *list, size_t index) {
    if (!list || index >= list->count) {
        return -1;
    }
    llist_node_t *current = list->head;
    llist_node_t *previous = NULL;
    size_t i = 0;
    while (current && i < index) {
        previous = current;
        current = current->next;
        i++;
    }
    if (!current) {
        return -1;
    }
    if (previous) {
        previous->next = current->next;
    } else {
        list->head = current->next;
    }
    if (current == list->tail) {
        list->tail = previous;
    }
    free(current->data);
    free(current);
    list->count--;
    return 0;
}

/*
* llist_clear:
*   Xóa toàn bộ các node trong danh sách, giữ nguyên cấu trúc danh sách.
*/
void llist_clear(linkedlist_t *list) {
    if (!list) {
        return;
    }
    llist_node_t *current = list->head;
    while (current) {
        llist_node_t *temp = current;
        current = current->next;
        free(temp->data);
        free(temp);
    }
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}

/*
* llist_destroy:
*   Hủy và giải phóng toàn bộ bộ nhớ của danh sách.
*/
void llist_destroy(linkedlist_t *list) {
    if (!list) {
        return;
    }
    llist_clear(list);
    free(list);
}

/*
* llist_get_first:
*   Lấy dữ liệu của node đầu tiên trong danh sách.
*/
void *llist_get_first(linkedlist_t *list) {
    if (list && list->head)
        return list->head->data;
    return NULL;
}

/*--------------------------------------------------------------------
* Linked List Iterator Functions
*--------------------------------------------------------------------*/

/*
* llist_iterator_init:
*   Khởi tạo iterator cho danh sách liên kết.
*/
void llist_iterator_init(linkedlist_t *list, linkedlist_iterator_t *it) {
    it->current = (list) ? list->head : NULL;
}

/*
* llist_iterator_has_next:
*   Kiểm tra xem iterator còn node nào không.
*/
int llist_iterator_has_next(linkedlist_iterator_t *it) {
    return (it->current != NULL);
}

/*
* llist_iterator_next:
*   Lấy dữ liệu của node tiếp theo từ iterator.
*/
void *llist_iterator_next(linkedlist_iterator_t *it) {
    if (it->current) {
        void *data = it->current->data;
        it->current = it->current->next;
        return data;
    }
    return NULL;
}
