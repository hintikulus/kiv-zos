#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <stdio.h>
#include "linkedlist.h"

typedef struct _file_system {
    char *file_name;
    FILE *file;
    struct superblock *sb;
    int32_t current_folder;
    linked_list *path;
} file_system;

#endif