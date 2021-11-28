//
// Created by hintik on 28.11.21.
//

#ifndef SP_FILESYSTEM_H
#define SP_FILESYSTEM_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "structures.h"
#include "inode.h"
#include "linkedlist.h"

file_system *file_system_open(char *file_name);

int file_system_format(file_system *fs, int size);

int file_system_create(FILE file, int size, int block_size);

int write_superblock(file_system *fs, struct superblock* sb);

int load_superblock(file_system *fs);

int file_system_close(file_system *fs0);

int print_folder_content(file_system *fs, int32_t folder);



#endif //SP_FILESYSTEM_H
