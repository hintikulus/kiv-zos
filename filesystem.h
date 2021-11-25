
#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "structures.h"
#include "inode.h"

file_system file_system_open(char *file_name);

int file_system_format(file_system *fs, int size);

int file_system_create(FILE file, int size, int block_size);

int write_superblock(file_system *fs, struct superblock* sb);

int file_system_close(file_system *fs0);

#endif