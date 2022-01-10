/**
 * Implementation of linked list
 */
#include "linkedlist.h"

/**
 * Function to create linked list
 * @return linkedlist structure
 */
linked_list *linked_list_create() {
    linked_list *list = NULL;

    list = (linked_list *) malloc(sizeof(linked_list));

    if(!list) {
        return NULL;
    }

    list->size = 0;
    list->first = NULL;
    list->last = NULL;

    return list;
}

/**
 * Function to add item to linked list
 * @param ls linkedlist structure
 * @param name item name
 * @return information about operation success
 */
int linked_list_add(linked_list *ls, char *name) {
    struct linked_list_item *item = NULL;
    int name_len;

    if(!name || !ls) {
        return EXIT_FAILURE;
    }

    item = (struct linked_list_item *) malloc(sizeof(struct linked_list_item));

    if(item == NULL) {
        return EXIT_FAILURE;
    }

    item->previous = NULL;
    item->next = NULL;

    name_len = strlen(name);

    char *tmp = (char *) malloc((name_len+1) * sizeof(char));

    if(tmp == NULL) {
        free(item);
        return EXIT_FAILURE;
    }

    item->name = tmp;

    strcpy(item->name, name);

    item->previous = ls->last;

    if(item->previous) {
        item->previous->next = item;
    }

    ls->last = item;

    if(ls->size == 0) {
        ls->first = item;
    }

    ls->size++;
    return EXIT_SUCCESS;
}

/**
 * Function to remove last item in linkedlist
 * @param ls linkedlist structure
 * @return information about operation success
 */
int linked_list_remove_last(linked_list *ls) {
    struct linked_list_item *item = NULL;

    if(!ls) {
        return EXIT_FAILURE;
    }

    if(ls->size == 0) {
        return EXIT_SUCCESS;
    }

    item = ls->last;

    if(!item) {
        return EXIT_FAILURE;
    }

    if(item->previous) {
        item->previous->next = NULL;
        ls->last = item->previous;
    } else {
        ls->last = NULL;
    }

    ls->size--;

    if(ls->size == 0) {
        ls->first = NULL;
        ls->last = NULL;
    }

    free(item->name);
    free(item);

    return EXIT_SUCCESS;
}

/**
 * Function to free allocated linked list
 * @param ls pointer to linkedlist structure
 * @return infromation about operation success
 */
int linked_list_free(linked_list **ls) {
    struct linked_list_item *item;

    if(!ls || !*ls) {
        return EXIT_FAILURE;
    }

    if(!(*ls)->first) {
        free(*ls);
        return EXIT_SUCCESS;
    }


    item = (*ls)->first;

    while(item->next) {
        item = item->next;
        free(item->previous->name);
        free(item->previous);
    }

    free(item->name);
    free(item);
    free(*ls);

    return EXIT_SUCCESS;
}

int linked_list_clear(linked_list *ls) {
    struct linked_list_item *item;

    item = ls->first;

    while(item->next) {
        item = item->next;
        free(item->previous->name);
        free(item->previous);
    }

    free(item->name);
    free(item);

    ls->first = NULL;
    ls->last = NULL;
    ls->size = 0;

    return EXIT_SUCCESS;

}