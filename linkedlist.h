//
// Created by hintik on 28.11.21.
//

#ifndef SP_LINKEDLIST_H
#define SP_LINKEDLIST_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <stdio.h>

/**
 * Structure of linked list accessible from both sides
 */
typedef struct linked_list_ {
    struct linked_list_item *first;
    struct linked_list_item *last;
    int32_t size;

} linked_list;

/**
 * Structure of linked list item accessible from both sides
 */
struct linked_list_item {
    char *name;
    struct linked_list_item *next;
    struct linked_list_item *previous;
};

/**
 * Function to create linked list
 * @return linkedlist structure
 */
linked_list *linked_list_create();

/**
 * Function to add item to linked list
 * @param ls linkedlist structure
 * @param name item name
 * @return information about operation success
 */
int linked_list_add(linked_list *ls, char *name);

/**
 * Function to remove last item in linkedlist
 * @param ls linkedlist structure
 * @return information about operation success
 */
int linked_list_remove_last(linked_list *ls);

/**
 * Function to free allocated linked list
 * @param ls pointer to linkedlist structure
 * @return infromation about operation success
 */
int linked_list_free(linked_list **ls);

#endif //SP_LINKEDLIST_H
