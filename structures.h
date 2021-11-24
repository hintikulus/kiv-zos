#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <stdio.h>

typedef struct _file_system {
    char *file_name;
    FILE *file;
    struct superblock *sb;
} file_system;

#endif