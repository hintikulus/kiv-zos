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

/**
 * Function to open filesystem
 * @param file_name path to file with filesystem
 * @return filesystem structure
 */
file_system *file_system_open(char *file_name);

/**
 * Function to format filesystem to entered size
 * @param path path to file
 * @param size size of filesystem in bytes
 * @return filesystem structure
 */
file_system *file_system_format(char *path, int size);

/**
 * Structure to write superblock to file
 * @param file opened file descriptor
 * @param sb pointer to superblock structure
 * @return
 */
int write_superblock(FILE *source_file, struct superblock* sb);

/**
 * Function to close filesystem
 * @param fs filesystem structure
 * @return information about success of operation
 */
int file_system_close(file_system *fs);

/**
 * Function to print content of folder
 * @param fs filesystem structure
 * @param folder folder identifier
 * @return information about success
 */
int print_folder_content(file_system *fs, int32_t folder);

/**
 * Function to fill datablock with data
 * @param fs filesystem structure
 * @param data data array
 * @param size data size
 * @return information about operation success
 */
int fill_datablock(file_system *fs, char *data, long size);

/**
 * Function to create file in filesystem from existing file
 * @param fs filesystem structure
 * @param source_file source file
 * @param path1 path to file
 * @param parent parent folder of the new file
 * @param name name of creating file
 * @return information of the operation success
 */
int create_file(file_system *fs, FILE *file, char *path1, int parent, char *name);

/**
 * Function to get filename from entered path
 * @param path filesystem path
 * @return pointer to position on path where starts file name
 */
char *get_name_from_path(char *path);

#endif //SP_FILESYSTEM_H
