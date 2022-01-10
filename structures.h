//
// Created by hintik on 28.11.21.
//

#ifndef SP_STRUCTURES_H
#define SP_STRUCTURES_H

#include <stdio.h>
#include "linkedlist.h"

#define DATABLOCK_SIZE 4096

/**
 * structure of filesystem
 */
typedef struct file_system_ {
    char *file_name;
    FILE *file;
    struct superblock *sb;
    int32_t current_folder;
    linked_list *path;
} file_system;

/**
 * Function to convert entered string value to byte size
 * @param size string size value
 * @return size in bytes
 */
int transfer_size(char *size);

/**
 * Function proccess one line command
 * @param fs structure of filesystem
 * @param cmd command string
 * @param path path to the file
 * @return information about success of the function
 */
int process_line(file_system **fs, char *cmd, char *path);
#endif //SP_STRUCTURES_H
