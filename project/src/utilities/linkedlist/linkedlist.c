#include "linkedlist.h"
#include <stdlib.h>

bool linkedlist_init(LinkedList* list, void (*free_data)(void*)) {
    if (!list) return false;
    
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    list->free_data = free_data;
    
    return true;
}

bool linkedlist_append(LinkedList* list, void* data) {
    if (!list) return false;
    
    ListNode* node = (ListNode*)malloc(sizeof(ListNode));
    if (!node) return false;
    
    node->data = data;
    node->next = NULL;
    
    if (list->tail) {
        list->tail->next = node;
        list->tail = node;
    } else {
        list->head = node;
        list->tail = node;
    }
    
    list->size++;
    return true;
}

bool linkedlist_prepend(LinkedList* list, void* data) {
    if (!list) return false;
    
    ListNode* node = (ListNode*)malloc(sizeof(ListNode));
    if (!node) return false;
    
    node->data = data;
    node->next = list->head;
    
    list->head = node;
    
    if (!list->tail) {
        list->tail = node;
    }
    
    list->size++;
    return true;
}

ListNode* linkedlist_get_node(LinkedList* list, uint32_t index) {
    if (!list || index >= list->size) return NULL;
    
    ListNode* current = list->head;
    for (uint32_t i = 0; i < index; i++) {
        current = current->next;
    }
    
    return current;
}

void* linkedlist_get(LinkedList* list, uint32_t index) {
    ListNode* node = linkedlist_get_node(list, index);
    return node ? node->data : NULL;
}

bool linkedlist_remove(LinkedList* list, uint32_t index) {
    if (!list || index >= list->size) return false;
    
    ListNode* to_remove;
    
    if (index == 0) {
        to_remove = list->head;
        list->head = list->head->next;
        
        if (list->head == NULL) {
            list->tail = NULL;
        }
    } else {
        ListNode* prev = linkedlist_get_node(list, index - 1);
        to_remove = prev->next;
        prev->next = to_remove->next;
        
        if (to_remove == list->tail) {
            list->tail = prev;
        }
    }
    
    if (list->free_data && to_remove->data) {
        list->free_data(to_remove->data);
    }
    
    free(to_remove);
    list->size--;
    
    return true;
}

void linkedlist_clear(LinkedList* list) {
    if (!list) return;
    
    ListNode* current = list->head;
    while (current) {
        ListNode* next = current->next;
        
        if (list->free_data && current->data) {
            list->free_data(current->data);
        }
        
        free(current);
        current = next;
    }
    
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

uint32_t linkedlist_size(LinkedList* list) {
    return list ? list->size : 0;
}

bool linkedlist_is_empty(LinkedList* list) {
    return list ? (list->size == 0) : true;
}

void linkedlist_foreach(LinkedList* list, void (*callback)(void* data, void* user_data), void* user_data) {
    if (!list || !callback) return;
    
    ListNode* current = list->head;
    while (current) {
        callback(current->data, user_data);
        current = current->next;
    }
}
