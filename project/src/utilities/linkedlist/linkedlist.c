/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Module Linkedlist cung cấp cấu trúc dữ liệu danh sách liên kết
 *   và các hàm để thao tác với danh sách.
 *********************************************************************/

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdio.h>
#include <string.h>
#include "linkedlist.h"

/*********************************************************************
 * Private Function Prototypes
 *********************************************************************/
static node_t *create_node(void *data);
static void free_node(node_t *node, free_fn free_func);

/*********************************************************************
 * Public Function Implementations
 *********************************************************************/

int32_t list_init(linkedlist_t *list, bool is_circular)
{
    if (list == NULL) {
        return LIST_INVALID;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    list->is_circular = is_circular;

    return LIST_SUCCESS;
}

int32_t list_push_front(linkedlist_t *list, void *data)
{
    if (list == NULL || data == NULL) {
        return LIST_INVALID;
    }

    node_t *new_node = create_node(data);
    if (new_node == NULL) {
        return LIST_NO_MEMORY;
    }

    if (list->head == NULL) {
        list->head = new_node;
        list->tail = new_node;
        if (list->is_circular) {
            new_node->next = new_node;
            new_node->prev = new_node;
        }
    } else {
        new_node->next = list->head;
        list->head->prev = new_node;
        list->head = new_node;
        if (list->is_circular) {
            new_node->prev = list->tail;
            list->tail->next = new_node;
        }
    }

    list->size++;
    return LIST_SUCCESS;
}

int32_t list_push_back(linkedlist_t *list, void *data)
{
    if (list == NULL || data == NULL) {
        return LIST_INVALID;
    }

    node_t *new_node = create_node(data);
    if (new_node == NULL) {
        return LIST_NO_MEMORY;
    }

    if (list->tail == NULL) {
        list->head = new_node;
        list->tail = new_node;
        if (list->is_circular) {
            new_node->next = new_node;
            new_node->prev = new_node;
        }
    } else {
        new_node->prev = list->tail;
        list->tail->next = new_node;
        list->tail = new_node;
        if (list->is_circular) {
            new_node->next = list->head;
            list->head->prev = new_node;
        }
    }

    list->size++;
    return LIST_SUCCESS;
}

int32_t list_pop_front(linkedlist_t *list, free_fn free_func)
{
    if (list == NULL || list->head == NULL) {
        return LIST_INVALID;
    }

    node_t *node = list->head;
    
    if (list->head == list->tail) {
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->head = node->next;
        if (list->is_circular) {
            list->head->prev = list->tail;
            list->tail->next = list->head;
        } else {
            list->head->prev = NULL;
        }
    }

    free_node(node, free_func);
    list->size--;

    return LIST_SUCCESS;
}

int32_t list_pop_back(linkedlist_t *list, free_fn free_func)
{
    if (list == NULL || list->tail == NULL) {
        return LIST_INVALID;
    }

    node_t *node = list->tail;
    
    if (list->head == list->tail) {
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->tail = node->prev;
        if (list->is_circular) {
            list->tail->next = list->head;
            list->head->prev = list->tail;
        } else {
            list->tail->next = NULL;
        }
    }

    free_node(node, free_func);
    list->size--;

    return LIST_SUCCESS;
}

int32_t list_insert(linkedlist_t *list, void *data, uint32_t index)
{
    if (list == NULL || data == NULL || index > list->size) {
        return LIST_INVALID;
    }

    if (index == 0) {
        return list_push_front(list, data);
    }

    if (index == list->size) {
        return list_push_back(list, data);
    }

    node_t *new_node = create_node(data);
    if (new_node == NULL) {
        return LIST_NO_MEMORY;
    }

    node_t *current = list->head;
    for (uint32_t i = 0; i < index - 1; i++) {
        current = current->next;
    }

    new_node->next = current->next;
    new_node->prev = current;
    current->next->prev = new_node;
    current->next = new_node;

    list->size++;
    return LIST_SUCCESS;
}

int32_t list_remove(linkedlist_t *list, uint32_t index, free_fn free_func)
{
    if (list == NULL || index >= list->size) {
        return LIST_INVALID;
    }

    if (index == 0) {
        return list_pop_front(list, free_func);
    }

    if (index == list->size - 1) {
        return list_pop_back(list, free_func);
    }

    node_t *current = list->head;
    for (uint32_t i = 0; i < index; i++) {
        current = current->next;
    }

    current->prev->next = current->next;
    current->next->prev = current->prev;

    free_node(current, free_func);
    list->size--;

    return LIST_SUCCESS;
}

node_t *list_find(linkedlist_t *list, const void *data, compare_fn compare_func)
{
    if (list == NULL || data == NULL || compare_func == NULL) {
        return NULL;
    }

    node_t *current = list->head;
    
    if (list->is_circular) {
        do {
            if (compare_func(current->data, data) == 0) {
                return current;
            }
            current = current->next;
        } while (current != list->head);
    } else {
        while (current != NULL) {
            if (compare_func(current->data, data) == 0) {
                return current;
            }
            current = current->next;
        }
    }

    return NULL;
}

int32_t list_clear(linkedlist_t *list, free_fn free_func)
{
    if (list == NULL) {
        return LIST_INVALID;
    }

    while (list->head != NULL) {
        list_pop_front(list, free_func);
    }

    return LIST_SUCCESS;
}

uint32_t list_size(const linkedlist_t *list)
{
    return (list != NULL) ? list->size : 0;
}

bool list_is_empty(const linkedlist_t *list)
{
    return (list != NULL) ? (list->size == 0) : true;
}

/*********************************************************************
 * Private Function Implementations
 *********************************************************************/

static node_t *create_node(void *data)
{
    node_t *node = (node_t *)malloc(sizeof(node_t));
    if (node != NULL) {
        node->data = data;
        node->next = NULL;
        node->prev = NULL;
    }
    return node;
}

static void free_node(node_t *node, free_fn free_func)
{
    if (node != NULL) {
        if (free_func != NULL) {
            free_func(node->data);
        }
        free(node);
    }
}

/*********************************************************************
 * UUID: 3a9d2f1b-4c4a-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/
