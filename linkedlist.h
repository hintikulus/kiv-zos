#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdlib.h>
#include <stdint.h>

typedef struct _linked_list {
    struct linked_list_item *first;
    struct linked_list_item *last;
    int32_t size;

} linked_list;

struct linked_list_item {
    char *name;
    struct linked_list_item *next;
    struct linked_list_item *previous;
};

linked_list *linked_list_create();

int linked_list_add(linked_list *ls, char *name);

int linked_list_remove_last(linked_list *ls);

int linked_list_free(linked_list *ls);



#endif