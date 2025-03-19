/**
 * @file linkedlist.h
 * @author Le Duc Son
 * @date 2018-11-21
 * @brief Header file for linked list
 */

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @struct ListNode
 * @brief Node structure for linked list
 */
typedef struct ListNode {
    void* data;                  /**< Pointer to data */
    struct ListNode* next;       /**< Pointer to next node */
} ListNode;

/**
 * @struct LinkedList
 * @brief Linked list structure
 */
typedef struct {
    ListNode* head;              /**< Pointer to first node */
    ListNode* tail;              /**< Pointer to last node */
    uint32_t size;               /**< Number of nodes in list */
    void (*free_data)(void*);     /**< Function to free data */
} LinkedList;

/**
 * @brief Initialize linked list
 * @param list Pointer to LinkedList structure
 * @param free_data Function to free data (can be NULL)
 * @return true if success, false if failed
 */
bool linkedlist_init(LinkedList* list, void (*free_data)(void*));

/**
 * @brief Append node to end of list
 * @param list Pointer to LinkedList structure
 * @param data Pointer to data
 * @return true if success, false if failed
 */
bool linkedlist_append(LinkedList* list, void* data);

/**
 * @brief Prepend node to beginning of list
 * @param list Pointer to LinkedList structure
 * @param data Pointer to data
 * @return true if success, false if failed
 */
bool linkedlist_prepend(LinkedList* list, void* data);

/**
 * @brief Get node at index
 * @param list Pointer to LinkedList structure
 * @param index Index of node to get
 * @return Pointer to node if success, NULL if failed
 */
ListNode* linkedlist_get_node(LinkedList* list, uint32_t index);

/**
 * @brief Get data at index
 * @param list Pointer to LinkedList structure
 * @param index Index of data to get
 * @return Pointer to data if success, NULL if failed
 */
void* linkedlist_get(LinkedList* list, uint32_t index);

/**
 * @brief Remove node at index
 * @param list Pointer to LinkedList structure
 * @param index Index of node to remove
 * @return true if success, false if failed
 */
bool linkedlist_remove(LinkedList* list, uint32_t index);

/**
 * @brief Clear linked list
 * @param list Pointer to LinkedList structure
 */
void linkedlist_clear(LinkedList* list);

/**
 * @brief Get size of linked list
 * @param list Pointer to LinkedList structure
 * @return Number of nodes in list
 */
uint32_t linkedlist_size(LinkedList* list);

/**
 * @brief Check if linked list is empty
 * @param list Pointer to LinkedList structure
 * @return true if list is empty, false if not
 */
bool linkedlist_is_empty(LinkedList* list);

/**
 * @brief Iterate linked list and apply callback for each node
 * @param list Pointer to LinkedList structure
 * @param callback Callback function
 * @param user_data User data to pass to callback
 */
void linkedlist_foreach(LinkedList* list, void (*callback)(void* data, void* user_data), void* user_data);

#endif // LINKEDLIST_H

