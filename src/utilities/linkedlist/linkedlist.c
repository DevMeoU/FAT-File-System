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
#include <stdlib.h>
#include "../log/print_color.h"

/*********************************************************************
 * Private Function Prototypes
 *********************************************************************/
static linkedlist_node_t *create_node(void *data);
static void free_node(linkedlist_node_t *node, free_fn free_func);

/*********************************************************************
 * Private Function Implementations
 *********************************************************************/

static linkedlist_node_t *create_node(void *data)
{
    linkedlist_node_t *node = (linkedlist_node_t *)malloc(sizeof(linkedlist_node_t));
    if (node == NULL) {
        print_error("Failed to allocate memory for node\n");
        return NULL;
    }

    node->data = data;
    node->next = NULL;

    return node;
}

static void free_node(linkedlist_node_t *node, free_fn free_func)
{
    if (node == NULL) {
        return;
    }

    if (free_func != NULL && node->data != NULL) {
        free_func(node->data);
    }

    free(node);
}

/*********************************************************************
 * Public Function Implementations
 *********************************************************************/

void linkedlist_init(linkedlist_t *list)
{
    if (list == NULL) {
        return;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

int32_t linkedlist_append(linkedlist_t *list, void *data)
{
    if (list == NULL || data == NULL) {
        return LIST_INVALID_PARAMETER;
    }

    linkedlist_node_t *new_node = create_node(data);
    if (new_node == NULL) {
        return LIST_ERROR;
    }

    if (list->head == NULL) {
        list->head = new_node;
        list->tail = new_node;
    } else {
        list->tail->next = new_node;
        list->tail = new_node;
    }

    list->size++;
    return LIST_SUCCESS;
}

int32_t linkedlist_prepend(linkedlist_t *list, void *data)
{
    if (list == NULL || data == NULL) {
        return LIST_INVALID_PARAMETER;
    }

    linkedlist_node_t *new_node = create_node(data);
    if (new_node == NULL) {
        return LIST_ERROR;
    }

    if (list->head == NULL) {
        list->head = new_node;
        list->tail = new_node;
    } else {
        new_node->next = list->head;
        list->head = new_node;
    }

    list->size++;
    return LIST_SUCCESS;
}

void *linkedlist_remove_first(linkedlist_t *list)
{
    if (list == NULL || list->head == NULL) {
        return NULL;
    }

    linkedlist_node_t *node = list->head;
    void *data = node->data;

    if (list->head == list->tail) {
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->head = node->next;
    }

    free_node(node, NULL);
    list->size--;

    return data;
}

void *linkedlist_remove_last(linkedlist_t *list)
{
    if (list == NULL || list->tail == NULL) {
        return NULL;
    }

    linkedlist_node_t *node = list->tail;
    void *data = node->data;

    if (list->head == list->tail) {
        list->head = NULL;
        list->tail = NULL;
    } else {
        linkedlist_node_t *current = list->head;
        while (current->next != list->tail) {
            current = current->next;
        }
        current->next = NULL;
        list->tail = current;
    }

    free_node(node, NULL);
    list->size--;

    return data;
}

uint32_t linkedlist_size(const linkedlist_t *list)
{
    if (list == NULL) {
        return 0;
    }

    return list->size;
}

bool linkedlist_is_empty(const linkedlist_t *list)
{
    if (list == NULL) {
        return true;
    }

    return list->size == 0;
}

void linkedlist_cleanup(linkedlist_t *list)
{
    if (list == NULL) {
        return;
    }

    linkedlist_clear(list, NULL);
}

linkedlist_node_t *linkedlist_find(const linkedlist_t *list, const void *data, compare_fn compare)
{
    if (list == NULL || data == NULL || compare == NULL) {
        return NULL;
    }

    linkedlist_node_t *current = list->head;
    while (current != NULL) {
        if (compare(current->data, data) == 0) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

void *linkedlist_remove(linkedlist_t *list, const void *data, compare_fn compare)
{
    if (list == NULL || data == NULL || compare == NULL) {
        return NULL;
    }

    linkedlist_node_t *current = list->head;
    linkedlist_node_t *prev = NULL;

    while (current != NULL) {
        if (compare(current->data, data) == 0) {
            void *removed_data = current->data;

            if (prev == NULL) {
                list->head = current->next;
            } else {
                prev->next = current->next;
            }

            if (current == list->tail) {
                list->tail = prev;
            }

            free_node(current, NULL);
            list->size--;

            return removed_data;
        }

        prev = current;
        current = current->next;
    }

    return NULL;
}

void linkedlist_clear(linkedlist_t *list, free_fn free_data)
{
    if (list == NULL) {
        return;
    }

    linkedlist_node_t *current = list->head;
    while (current != NULL) {
        linkedlist_node_t *next = current->next;
        free_node(current, free_data);
        current = next;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

/*********************************************************************
 * UUID: 8b9c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/
