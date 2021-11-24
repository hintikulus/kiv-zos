#ifndef INODE_H
#define INODE_H

// priklad - verze 2021-01
// jedná se o SIMULACI souborového systému
// první i-uzel bude odkaz na hlavní adresář (1. datový blok)
// počet přímých i nepřímých odkazů je v reálném systému větší
// adresář vždy obsahuje dvojici číslo i-uzlu - název souboru
// jde jen o příklad, vlastní datové struktury si můžete upravit

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "structures.h"
#include "string.h"
#include "filesystem.h"

#define DIRECT_LINKS_COUNT 5

struct superblock {
    char signature[9];              //login autora FS
    char volume_descriptor[251];    //popis vygenerovaného FS
    int32_t disk_size;              //celkova velikost VFS
    int32_t datablock_size;           //velikost clusteru
    int32_t datablock_count;          //pocet clusteru
    int32_t inode_count;
    int32_t bitmapi_start_address;  //adresa pocatku bitmapy i-uzlů
    int32_t bitmap_start_address;   //adresa pocatku bitmapy datových bloků
    int32_t inode_start_address;    //adresa pocatku  i-uzlů
    int32_t data_start_address;     //adresa pocatku datovych bloku  
};


struct pseudo_inode {
    int32_t nodeid;                 //ID i-uzlu, pokud ID = ID_ITEM_FREE, je polozka volna
    bool isDirectory;               //soubor, nebo adresar
    int8_t references;              //počet odkazů na i-uzel, používá se pro hardlinky
    int32_t file_size;              //velikost souboru v bytech
    int32_t direct[DIRECT_LINKS_COUNT];                // 1. přímý odkaz na datové bloky
    //int32_t direct2;                // 2. přímý odkaz na datové bloky
    //int32_t direct3;                // 3. přímý odkaz na datové bloky
    //int32_t direct4;                // 4. přímý odkaz na datové bloky
    //int32_t direct5;                // 5. přímý odkaz na datové bloky
    int32_t indirect1;              // 1. nepřímý odkaz (odkaz - datové bloky)
    int32_t indirect2;              // 2. nepřímý odkaz (odkaz - odkaz - datové bloky)
};


struct directory_item {
    int32_t inode;                   // inode odpovídající souboru
    char item_name[12];              //8+3 + /0 C/C++ ukoncovaci string znak
};

// funkce pro predvyplneni struktury sb

void fill_default_sb(struct superblock *sb);
// funkce pro predvyplneni struktury sb

void fill_superblock(struct superblock *sb, int32_t size, int32_t datablock_size);



int get_free_inode_id(file_system *fs);

int get_free_datablock_id(file_system *fs);

int create_inode(file_system *fs);

int create_datablock(file_system *fs);

int create_root_directory(file_system *fs);

int create_directory(file_system *fs, int32_t parent, char *name);
int find_free_directory_item_in_datablock(file_system *fs, int32_t datablock);
int set_file_inode_position(file_system *fs, int32_t inode_id);
int set_file_datablock_position(file_system *fs, int32_t datablock_id);
int find_directory_item_in_datablock(file_system *fs, int32_t datablock, char *name);

int get_directory_item_inode(file_system *fs, int32_t parent, char* name);

int32_t get_inode_by_path(file_system *fs, int32_t parent, char *path);

int free_inode(file_system *fs, int32_t inode_id);

int free_datablock(file_system *fs, int32_t datablock_id);
#endif