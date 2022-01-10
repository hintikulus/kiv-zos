//
// Created by hintik on 28.11.21.
//

#ifndef SP_INODE_H
#define SP_INODE_H

/*
 priklad - verze 2021-01
 jedná se o SIMULACI souborového systému
 první i-uzel bude odkaz na hlavní adresář (1. datový blok)
 počet přímých i nepřímých odkazů je v reálném systému větší
 adresář vždy obsahuje dvojici číslo i-uzlu - název souboru
 jde jen o příklad, vlastní datové struktury si můžete upravit
*/

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "structures.h"
#include "string.h"
#include "filesystem.h"

#define DIRECT_LINKS_COUNT 5
#define INDIRECT_LINKS_COUNT 2

/**
 * Structure of superblock
 */
struct superblock {
    /* author's login */
    char signature[9];
    /* volume information */
    char volume_descriptor[251];
    /* datablock size */
    int32_t datablock_size;
    /* disk size */
    int32_t disk_size;
    /* datablock_count */
    int32_t datablock_count;
    /* inode_count */
    int32_t inode_count;
    /*
     * adresses of start of each block
     */
    int32_t bitmapi_start_address;
    int32_t bitmap_start_address;
    int32_t inode_start_address;
    int32_t data_start_address;
};

/**
 * Structure of inode
 */
struct pseudo_inode {
    /* file identifier */
    int32_t nodeid;
    /* is this a file or directory */
    bool isDirectory;
    /* number of directory items pointing to this inode */
    int8_t references;
    /* size of this file in bytes */
    long file_size;
    /* identifiers of direct datatblocks */
    int32_t direct[DIRECT_LINKS_COUNT];
    /* identifiers of indirect datablocks */
    int32_t indirect[INDIRECT_LINKS_COUNT];
};

/**
 * Structure for directory item
 */
struct directory_item {
    /* inode identifier */
    int32_t inode;
    /* file name */
    char item_name[12];
};

/**
 * Function to calculate addresses and fill the superblock
 * @param sb superblock structure
 * @param size size of filesystem
 * @param datablock_size size of one datablock
 */
void fill_superblock(struct superblock *sb, int32_t size, int32_t datablock_size);

/**
 * Function to allocate space for new inode
 * @param fs filesystem structure
 * @return inode identifier
 */
int get_free_inode_id(file_system *fs);

/**
 * Function to allocate space for new datablock
 * @param fs filesystem structure
 * @return datablock identifier
 */
int get_free_datablock_id(file_system *fs);

/**
 * Function to initialize filesystem root directory
 * @param fs filesystem structure
 * @return information about operation success
 */
int create_root_directory(file_system *fs);

/**
 * Function to create directory in certain folder
 * @param fs filesystem structure
 * @param parent folder inode identifier
 * @param name filename
 * @return information about operation success
 */
int create_directory(file_system *fs, int32_t parent, char *name);

/**
 * Function to find free directory item in certain datablock
 * @param fs filesystem structure
 * @param datablock datablock structure
 * @return position of free directory item
 */
int find_free_directory_item_in_datablock(file_system *fs, int32_t datablock);

/**
 * Function to find free directory item in certain folder
 * @param fs filesystem structure
 * @param folder folder inode identifier
 * @param datablock_number pointer to save datablock identifier
 * @return position of free directory item in datablock
 */
int find_free_directory_item_in_folder(file_system *fs, int32_t folder, int32_t *datablock_number);

/**
 * Function to set pointer in file to start of entered inode
 * @param fs filesystem structure
 * @param inode_id inode identifier
 * @return information about operation success
 */
int set_file_inode_position(file_system *fs, int32_t inode_id);

/**
 * Function to set pointer in file to start of entered datablock
 * @param fs filesystem structure
 * @param datablock_id datablock identifier
 * @return information about operation success
 */
int set_file_datablock_position(file_system *fs, int32_t datablock_id);

/**
 * Function to find directory item in datablock
 * @param fs filesystem structure
 * @param datablock datablock identifier
 * @param name file name
 * @return inode identifier of found file
 */
int find_directory_item_in_datablock(file_system *fs, int32_t datablock, char *name);


/**
 * Function to get new datablock for inode
 * @param fs filesystem structure
 * @param inode_id inode identifier
 * @return new datablock identifier
 */
int get_new_inode_datablock(file_system *fs, int inode_id);

/**
 * Function to get number of inode from entered filesystem path
 * @param fs filesystem structure
 * @param parent inode identifier from which the path starts
 * @param path filesystem path
 * @param offset where we should stop finding
 * @return identifier to targeting inode
 */
int32_t get_inode_by_path(file_system *fs, int32_t parent, char *path, int offset);

/**
 * Function to find directory item in filder
 * @param fs filesystem structure
 * @param folder folder inode identifier
 * @param name file name
 * @return inode identifier of found file
 */
int32_t find_file_in_folder(file_system *fs, int32_t folder, char *name);

/**
 * Function to set directorey item in folder
 * @param fs filesystem structure
 * @param parent folder inode identifier
 * @param inode inode item identifier
 * @param name item name
 * @return information about operation success
 */
int set_directory_item(file_system *fs, int32_t parent, int32_t inode, char *name);

/**
 * Function to translate inode datablock address to filesystem datablock address
 * @param fs filesystem structure
 * @param inode inode structure
 * @param datablock_id datablock inode-address
 * @return filesystem datablock identifier
 */
int32_t get_datablock_id(file_system *fs, struct pseudo_inode *inode, int32_t datablock_id);

/**
 * Function to assign datablock to inode
 * @param fs filesystem structure
 * @param inode inode structure
 * @param datablock_id inode-address of datablock
 * @param datablock_address datablock identifier
 * @return information about operation success
 */
int set_datablock_id(file_system *fs, struct pseudo_inode *inode, int32_t datablock_id, int32_t datablock_address);

/**
 * Function to remove directory item from datablock
 * @param fs filesystem structure
 * @param datablock datablock identifier
 * @param name filename
 * @return datablock inode-address where directory item was found
 */
int unset_directory_item_in_datablock(file_system *fs, int32_t datablock, char *name);

/**
 * Function to remove directory item from folder
 * @param fs filesystem structure
 * @param folder folder inode identifier
 * @param name file name
 * @return datablock inode-address where directory item was found
 */
int unset_directory_item_in_folder(file_system *fs, int32_t folder, char *name);

/**
 * Function to remove directory item from given path
 * @param fs filesystem structure
 * @param path path to directory item
 * @return information about operation success
 */
int unset_directory_item(file_system *fs, char *path);

/**
 * Function to deallocate allocated inode
 * @param fs filesystem structure
 * @param inode_id inode identifier
 * @return information about operation success
 */
int free_inode(file_system *fs, int32_t inode_id);
/**
 * Function to deallocate allocated datablock
 * @param fs filesystem structure
 * @param datablock_id datablock identifier
 * @return information about operation success
 */
int free_datablock(file_system *fs, int32_t datablock_id);


/**
 * Function to load inode structure from filesystem
 * @param fs filesystem structure
 * @param inode_id inode identifier
 * @param inode inode structure
 * @return information about operation success
 */
int load_inode(file_system *fs, int32_t inode_id, struct pseudo_inode *inode);

/**
 * Function to save inode structure to filesystem
 * @param fs filesystem structure
 * @param inode_id inode identifier
 * @param inode inode structure
 * @return information about operation success
 */
int save_inode(file_system *fs, int32_t inode_id, struct pseudo_inode *inode);

#endif //SP_INODE_H
