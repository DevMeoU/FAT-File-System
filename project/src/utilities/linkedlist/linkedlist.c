/**
 * @file linkedlist.c
 * @brief Implementation of the LinkedList structure and functions
 * @author Le Duc Son
 * @date 2022-04-20
 */

#include "linkedlist.h"
#include <stdlib.h>

/**
 * Initialize a LinkedList
 * @param list the LinkedList to initialize
 * @param free_data the function to free data in the list
 * @return true if successful, false otherwise
 */
bool linkedlist_init(LinkedList* list, void (*free_data)(void*)) {
    if (!list) return false;
    
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    list->free_data = free_data;
    
    return true;
}

/**
 * Append a node to the end of the list
 * @param list the LinkedList to append to
 * @param data the data to store in the node
 * @return true if successful, false otherwise
 */
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

/**
 * Prepend a node to the beginning of the list
 * @param list the LinkedList to prepend to
 * @param data the data to store in the node
 * @return true if successful, false otherwise
 */
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

/**
 * Get a node at a specific index
 * @param list the LinkedList to get from
 * @param index the index of the node
 * @return the node at the index, or NULL if out of bounds
 */
ListNode* linkedlist_get_node(LinkedList* list, uint32_t index) {
    if (!list || index >= list->size) return NULL;
    
    ListNode* current = list->head;
    for (uint32_t i = 0; i < index; i++) {
        current = current->next;
    }
    
    return current;
}

/**
 * Get the data of a node at a specific index
 * @param list the LinkedList to get from
 * @param index the index of the node
 * @return the data of the node at the index, or NULL if out of bounds
 */
void* linkedlist_get(LinkedList* list, uint32_t index) {
    ListNode* node = linkedlist_get_node(list, index);
    return node ? node->data : NULL;
}

/**
 * Remove a node at a specific index
 * @param list the LinkedList to remove from
 * @param index the index of the node to remove
 * @return true if successful, false otherwise
 */
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

/**
 * Clear the LinkedList
 * @param list the LinkedList to clear
 */
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

/**
 * Get the size of the LinkedList
 * @param list the LinkedList to get the size of
 * @return the size of the LinkedList
 */
uint32_t linkedlist_size(LinkedList* list) {
    return list ? list->size : 0;
}

/**
 * Check if the LinkedList is empty
 * @param list the LinkedList to check
 * @return true if the LinkedList is empty, false otherwise
 */
bool linkedlist_is_empty(LinkedList* list) {
    return list ? (list->size == 0) : true;
}

/**
 * Iterate over the LinkedList
 * @param list the LinkedList to iterate over
 * @param callback the function to call for each node
 * @param user_data the user data to pass to the callback
 */
void linkedlist_foreach(LinkedList* list, void (*callback)(void* data, void* user_data), void* user_data) {
    if (!list || !callback) return;
    
    ListNode* current = list->head;
    while (current) {
        callback(current->data, user_data);
        current = current->next;
    }
}

