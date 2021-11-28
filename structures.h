//
// Created by hintik on 28.11.21.
//

#ifndef SP_STRUCTURES_H
#define SP_STRUCTURES_H

#include <stdio.h>
#include "linkedlist.h"

typedef struct _file_system {
    char *file_name;
    FILE *file;
    struct superblock *sb;
    int32_t current_folder;
    linked_list *path;
} file_system;

#endif //SP_STRUCTURES_H
