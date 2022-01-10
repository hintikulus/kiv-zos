#include "inode.h"

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

const int32_t ID_ITEM_FREE = 0;

/**
 * Function to calculate addresses and fill the superblock
 * @param sb superblock structure
 * @param size size of filesystem
 * @param datablock_size size of one datablock
 */
void fill_superblock(struct superblock *sb, int32_t size, int32_t datablock_size) {
    memset(sb, 0, sizeof(struct superblock));
    strcpy(sb->signature, "hintik");
    strcpy(sb->volume_descriptor, "Semestralni prace KIV/ZOS");
    sb->disk_size = size;

    sb->datablock_size = datablock_size;
    size = size - sizeof(struct superblock);
    double bytes_per_inode = datablock_size;
    int inodes = ceil(size / bytes_per_inode);
    sb->inode_count = inodes;
    size = size - inodes * sizeof(struct pseudo_inode);
    size = size - ceil(inodes / 8.0);
    sb->datablock_count = size / datablock_size;
    size = size - ceil(sb->datablock_count / 8.0);
    size = size - sb->datablock_count * datablock_size;

    sb->bitmapi_start_address = sizeof(struct superblock);
    sb->bitmap_start_address = sb->bitmapi_start_address + ceil(inodes / 8.0);

    sb->inode_start_address = sb->bitmap_start_address + ceil(sb->datablock_count / 8.0);

    sb->data_start_address = sb->inode_start_address + inodes * sizeof(struct pseudo_inode);
}

/**
 * Function to allocate space for new inode
 * @param fs filesystem structure
 * @return inode identifier
 */
int get_free_inode_id(file_system *fs) {
    if(!fs) {
        return 0;
    }

    int array_size = fs->sb->bitmap_start_address - fs->sb->bitmapi_start_address;
    u_char inode_bitmap[array_size];
    memset(inode_bitmap, 0, array_size * sizeof(u_char));

    fseek(fs->file, fs->sb->bitmapi_start_address, SEEK_SET);

    fread(inode_bitmap, sizeof(u_char) * fs->sb->bitmap_start_address - fs->sb->bitmapi_start_address, 1, fs->file);

    for (int i = 0 ; i < ceil(fs->sb->inode_count/8.0); i++) {
        for (int j=7; j>=0; j--) {
            if(i * 8 + j >= fs->sb->inode_count) {
                continue;
            }

            if(((inode_bitmap[i] >> j) & 0b1) == 0b0) {
                fseek(fs->file, fs->sb->bitmapi_start_address + i * sizeof(u_char), SEEK_SET);
                inode_bitmap[i] |= 1UL << j;
                fwrite(&inode_bitmap[i], sizeof(u_char), 1, fs->file);
                return i*8+8-j;
            }
        }
    }
    return 0;
}

/**
 * Function to allocate space for new datablock
 * @param fs filesystem structure
 * @return datablock identifier
 */
int get_free_datablock_id(file_system *fs) {

    u_char data_bitmap[fs->sb->inode_start_address - fs->sb->bitmap_start_address];
    memset(data_bitmap, '\0', sizeof(data_bitmap));
    fseek(fs->file, fs->sb->bitmap_start_address, SEEK_SET);
    fread(data_bitmap, sizeof(data_bitmap), 1, fs->file);

    for (int i = 0 ; i < ceil(fs->sb->datablock_count/8.0); i++) {
        for (int j=7; j>=0; j--) {
            if(i * 8 + j >= fs->sb->datablock_count) {
                continue;
            }
            if(((data_bitmap[i] >> j) & 0b1) == 0b0) {
                fseek(fs->file, fs->sb->bitmap_start_address + i * sizeof(u_char), SEEK_SET);
                data_bitmap[i] |= 1UL << j;
                fwrite(&data_bitmap[i], sizeof(u_char), 1, fs->file);
                return i*8+8-j;
            }
        }
    }

    return 0;
}

/**
 * Function to initialize filesystem root directory
 * @param fs filesystem structure
 * @return information about operation success
 */
int create_root_directory(file_system *fs) {
    int i;
    int inode_id = get_free_inode_id(fs);
    int data_id = get_free_datablock_id(fs);

    if(inode_id == 0 || data_id == 0) {
        return EXIT_FAILURE;
    }

    struct pseudo_inode inode = {};
    (&inode)->isDirectory = true;
    (&inode)->nodeid = inode_id;
    (&inode)->references = 0;
    (&inode)->file_size = fs->sb->datablock_size;

    (&inode)->direct[0] = data_id;

    for(i = 1; i < DIRECT_LINKS_COUNT; i++) {
        inode.direct[i] = 0;
    }

    for(i = 1; i < INDIRECT_LINKS_COUNT; i++) {
        inode.indirect[i] = 0;
    }

    set_file_inode_position(fs, inode_id);
    fwrite(&inode, sizeof(inode), 1, fs->file);

    set_file_datablock_position(fs, data_id);
    struct directory_item di_root = {inode_id, "."};
    fwrite(&di_root, sizeof(struct directory_item), 1, fs->file);
    struct directory_item di_up = {inode_id, ".."};
    fwrite(&di_up, sizeof(struct directory_item), 1, fs->file);

    return EXIT_SUCCESS;
}

/**
 * Function to create directory in certain folder
 * @param fs filesystem structure
 * @param parent folder inode identifier
 * @param name filename
 * @return information about operation success
 */
int create_directory(file_system *fs, int32_t parent, char *name) {
    if(parent < 1) {
        return 2;
    }

    struct pseudo_inode parent_inode;
    //int i;
    int32_t free_directory_item, free_directory_item_block;
    int32_t inode_id;
    set_file_inode_position(fs, parent);
    fread(&parent_inode, sizeof(struct pseudo_inode), 1, fs->file);
    free_directory_item_block = 0;
    free_directory_item = 0;

    bool isD = (&parent_inode)->isDirectory;

    if(!isD) {
        return 4;
    }


    inode_id = find_file_in_folder(fs, parent, name);
    if(inode_id != 0) {
        return 3;
    }

    inode_id = get_free_inode_id(fs);
    int32_t data_id = get_free_datablock_id(fs);

    set_directory_item(fs, parent, inode_id, name);

    if(inode_id == 0 || data_id == 0) {
        return EXIT_FAILURE;
    }

    struct pseudo_inode inode = {};
    (&inode)->isDirectory = true;
    (&inode)->nodeid = inode_id;
    (&inode)->file_size = fs->sb->datablock_size;
    (&inode)->references = 0;
    (&inode)->direct[0] = data_id;
    (&inode)->direct[1] = 0;
    (&inode)->direct[2] = 0;
    (&inode)->direct[3] = 0;
    (&inode)->direct[4] = 0;
    inode.indirect[0] = 0;
    inode.indirect[1] = 0;

    save_inode(fs, inode_id, &inode);

    set_file_datablock_position(fs, data_id);
    struct directory_item di_root = {inode_id, "."};
    fwrite(&di_root, sizeof(struct directory_item), 1, fs->file);
    struct directory_item di_up = {parent, ".."};
    fwrite(&di_up, sizeof(struct directory_item), 1, fs->file);

    struct directory_item item = {};
    (&item)->inode = inode_id;
    strcpy((&item)->item_name, name);

    return EXIT_SUCCESS;
}

/**
 * Function to find free directory item in certain folder
 * @param fs filesystem structure
 * @param folder folder inode identifier
 * @param datablock_number pointer to save datablock identifier
 * @return position of free directory item in datablock
 */
int find_free_directory_item_in_folder(file_system *fs, int32_t folder, int32_t *datablock_number) {
    int i, j, k;
    int32_t link, found_id;
    struct pseudo_inode parent_inode;
    int count = fs->sb->datablock_size / sizeof(int32_t);
    //int32_t items_in_block = (fs->sb->datablock_size / sizeof(struct directory_item));

    set_file_inode_position(fs, folder);
    fread(&parent_inode, sizeof(struct pseudo_inode), 1, fs->file);

    for(i = 0; i < DIRECT_LINKS_COUNT; i++) {
        if((&parent_inode)->direct[i] != 0) {
            found_id = find_free_directory_item_in_datablock(fs, parent_inode.direct[i]);
            if(found_id != 0) {

                *datablock_number = parent_inode.direct[i];
                return found_id;
            }

        }
    }

    /*
       Procházení nepřímých odkazů

       1. - datablock - slozka
       2. - datablock - datablock - slozka
    */

    int32_t datablock[INDIRECT_LINKS_COUNT][count];

    link = parent_inode.indirect[0];

    if(link != 0) {
        set_file_datablock_position(fs, link);
        fread(datablock[0], fs->sb->datablock_size, 1, fs->file);

        for(k = 0; k < count; k++) {
            link = datablock[0][k];
            found_id = find_free_directory_item_in_datablock(fs, link);
            if(found_id != 0) {
                *datablock_number = link;
                return found_id;
            }
        }
    }

    link = parent_inode.indirect[1];
    if(link != 0) {
        set_file_datablock_position(fs, link);
        fread(datablock[0], fs->sb->datablock_size, 1, fs->file);

        for(k = 0; k < count; k++) {
            link = datablock[0][k];
            if(link != 0) {
                set_file_datablock_position(fs, link);
                fread(datablock[1], fs->sb->datablock_size, 1, fs->file);

                for(j = 0; j < count; j++) {
                    link = datablock[1][j];
                    if(link != 0) {
                        found_id = find_free_directory_item_in_datablock(fs, link);
                        if(found_id != 0) {
                            *datablock_number = link;
                            return found_id;
                        }
                    }
                }
            }

        }
    }
    return 0;
}

/**
 * Function to find free directory item in certain datablock
 * @param fs filesystem structure
 * @param datablock datablock structure
 * @return position of free directory item
 */
int find_free_directory_item_in_datablock(file_system *fs, int32_t datablock) {
    int i;
    int item_count = fs->sb->datablock_size/sizeof(struct directory_item);
    struct directory_item items[item_count];

    set_file_datablock_position(fs, datablock);
    fread(items, sizeof(items), 1, fs->file);
    for(i = 0; i < item_count; i++) {
        if((&items[i])->inode == 0) {
            return i;
        }
    }

    return 0;
}

/**
 * Function to remove directory item from given path
 * @param fs filesystem structure
 * @param path path to directory item
 * @return information about operation success
 */
int unset_directory_item(file_system *fs, char *path) {
    char name[12] = "";
    strcpy(name, get_name_from_path(path));
    if(!strcmp(name, ".") || !strcmp(name, "..")) {
        return EXIT_FAILURE;
    }
    int parent = get_inode_by_path(fs, fs->current_folder, path, 1);
    if(unset_directory_item_in_folder(fs, parent, name) > 0) {
        struct pseudo_inode inode = {};
        load_inode(fs, parent, &inode);
        (&inode)->references--;
        save_inode(fs, parent, &inode);
    };

    return EXIT_SUCCESS;
}

/**
 * Function to remove directory item from folder
 * @param fs filesystem structure
 * @param folder folder inode identifier
 * @param name file name
 * @return datablock inode-address where directory item was found
 */
int unset_directory_item_in_folder(file_system *fs, int32_t folder, char *name) {

    struct pseudo_inode parent_inode;
    int i, j, k;
    int32_t found_id, link;
    int count = fs->sb->datablock_size / sizeof(int32_t);

    set_file_inode_position(fs, folder);
    fread(&parent_inode, sizeof(struct pseudo_inode), 1, fs->file);

    for(i = 0; i < DIRECT_LINKS_COUNT; i++) {
        if((&parent_inode)->direct[i] != 0) {
            found_id = unset_directory_item_in_datablock(fs, parent_inode.direct[i], name);
            if(found_id != 0) {
                return found_id;
            }

        }
    }

    /*
       Procházení nepřímých odkazů

       1. - datablock - slozka
       2. - datablock - datablock - slozka
    */

    int32_t datablock[INDIRECT_LINKS_COUNT][count];

    link = parent_inode.indirect[0];

    if(link != 0) {
        set_file_datablock_position(fs, link);
        fread(datablock[0], fs->sb->datablock_size, 1, fs->file);

        for(k = 0; k < count; k++) {
            link = datablock[0][k];
            found_id = unset_directory_item_in_datablock(fs, link, name);
            if(found_id != 0) {
                return found_id;
            }
        }
    }

    link = parent_inode.indirect[1];
    if(link != 0) {
        set_file_datablock_position(fs, link);
        fread(datablock[0], fs->sb->datablock_size, 1, fs->file);

        for(k = 0; k < count; k++) {
            link = datablock[0][k];
            if(link != 0) {
                set_file_datablock_position(fs, link);
                fread(datablock[1], fs->sb->datablock_size, 1, fs->file);

                for(j = 0; j < count; j++) {
                    link = datablock[1][j];
                    if(link != 0) {
                        found_id = unset_directory_item_in_datablock(fs, link, name);
                        if(found_id != 0) {
                            return found_id;
                        }
                    }
                }
            }

        }
    }

    return 0;
}

/**
 * Function to remove directory item from datablock
 * @param fs filesystem structure
 * @param datablock datablock identifier
 * @param name filename
 * @return datablock inode-address where directory item was found
 */
int unset_directory_item_in_datablock(file_system *fs, int32_t datablock, char *name) {

    int i;
    int item_count = fs->sb->datablock_size/sizeof(struct directory_item);
    struct directory_item items[item_count];

    set_file_datablock_position(fs, datablock);
    fread(items, sizeof(items), 1, fs->file);
    for(i = 0; i < item_count; i++) {
        if(strcmp((&items[i])->item_name, name) == 0) {
            set_file_datablock_position(fs, datablock);
            fseek(fs->file, sizeof(struct directory_item) * i, SEEK_CUR);
            char zero = '\0';
            for(i = 0; i < sizeof(struct directory_item); i++) {
                fwrite(&zero, sizeof(char), 1, fs->file);
            }
            return i;
        }
    }

    return 0;
}

/**
 * Function to find directory item in datablock
 * @param fs filesystem structure
 * @param datablock datablock identifier
 * @param name file name
 * @return inode identifier of found file
 */
int find_directory_item_in_datablock(file_system *fs, int32_t datablock, char *name) {
    int i;
    int item_count = fs->sb->datablock_size/sizeof(struct directory_item);
    struct directory_item items[item_count];

    set_file_datablock_position(fs, datablock);
    fread(items, sizeof(items), 1, fs->file);
    for(i = 0; i < item_count; i++) {
        if(strcmp((&items[i])->item_name, name) == 0) {
            return (&items[i])->inode;
        }
    }

    return 0;

}

/**
 * Function to find directory item in filder
 * @param fs filesystem structure
 * @param folder folder inode identifier
 * @param name file name
 * @return inode identifier of found file
 */
int32_t find_file_in_folder(file_system *fs, int32_t folder, char *name) {
    struct pseudo_inode parent_inode;
    int i, j, k;
    int32_t found_id, link;
    int count = fs->sb->datablock_size / sizeof(int32_t);

    set_file_inode_position(fs, folder);
    fread(&parent_inode, sizeof(struct pseudo_inode), 1, fs->file);

    for(i = 0; i < DIRECT_LINKS_COUNT; i++) {
        if((&parent_inode)->direct[i] != 0) {
            found_id = find_directory_item_in_datablock(fs, parent_inode.direct[i], name);
            if(found_id != 0) {
                return found_id;
            }

        }
    }

    /*
       Procházení nepřímých odkazů

       1. - datablock - slozka
       2. - datablock - datablock - slozka
    */

    int32_t datablock[INDIRECT_LINKS_COUNT][count];

    link = parent_inode.indirect[0];

    if(link != 0) {
        set_file_datablock_position(fs, link);
        fread(datablock[0], fs->sb->datablock_size, 1, fs->file);

        for(k = 0; k < count; k++) {
            link = datablock[0][k];
            found_id = find_directory_item_in_datablock(fs, link, name);
            if(found_id != 0) {
                return found_id;
            }
        }
    }

    link = parent_inode.indirect[1];
    if(link != 0) {
        set_file_datablock_position(fs, link);
        fread(datablock[0], fs->sb->datablock_size, 1, fs->file);

        for(k = 0; k < count; k++) {
            link = datablock[0][k];
            if(link != 0) {
                set_file_datablock_position(fs, link);
                fread(datablock[1], fs->sb->datablock_size, 1, fs->file);

                for(j = 0; j < count; j++) {
                    link = datablock[1][j];
                    if(link != 0) {
                        found_id = find_directory_item_in_datablock(fs, link, name);
                        if(found_id != 0) {
                            return found_id;
                        }
                    }
                }
            }

        }
    }

    return 0;

}

/**
 * Function to get new datablock for inode
 * @param fs filesystem structure
 * @param inode_id inode identifier
 * @return new datablock identifier
 */
int get_new_inode_datablock(file_system *fs, int inode_id) {
    struct pseudo_inode inode = {};
    load_inode(fs, inode_id, &inode);


    int n = fs->sb->datablock_size / sizeof(int32_t);
    int k = DIRECT_LINKS_COUNT + n + n * n;

    int i;
    for(i = 0; i < k; i++) {
        int db_id = get_datablock_id(fs, &inode, i);
        if(db_id == 0) {
            int new_db_id = get_free_datablock_id(fs);
            if(new_db_id <= 0) {
                return 0;
            }

            set_datablock_id(fs, &inode, i, new_db_id);

            (&inode)->file_size += fs->sb->datablock_size;
            save_inode(fs, inode_id, &inode);

            return new_db_id;
        }
    }

}

/**
 * Function to set directorey item in folder
 * @param fs filesystem structure
 * @param parent folder inode identifier
 * @param inode inode item identifier
 * @param name item name
 * @return information about operation success
 */
int set_directory_item(file_system *fs, int32_t parent, int32_t inode, char *name) {
    int datablock_number = 0;
    int free_directory_item = find_free_directory_item_in_folder(fs, parent, &datablock_number);

    if(free_directory_item == 0 || datablock_number == 0) {
        get_new_inode_datablock(fs, parent);
        free_directory_item = find_free_directory_item_in_folder(fs, parent, &datablock_number);
        if(free_directory_item == 0 || datablock_number == 0) {
            //Neni misto
            return EXIT_FAILURE;
        }
    }

    struct directory_item di = {};
    (&di)->inode = inode;
    strncpy((&di)->item_name, name, 11);

    struct pseudo_inode parent_inode = { 0 };
    load_inode(fs, parent, &parent_inode);
    (&parent_inode)->references = (&parent_inode)->references + 1;
    save_inode(fs, parent, &parent_inode);

    set_file_datablock_position(fs, datablock_number);
    fseek(fs->file, free_directory_item * sizeof(struct directory_item), SEEK_CUR);
    fwrite(&di, sizeof(struct directory_item), 1, fs->file);

    fflush(fs->file);

}

/**
 * Function to set pointer in file to start of entered inode
 * @param fs filesystem structure
 * @param inode_id inode identifier
 * @return information about operation success
 */
int set_file_inode_position(file_system *fs, int32_t inode_id) {
    if(!fs || !fs->sb) {
        return EXIT_FAILURE;
    }
    return fseek(fs->file, fs->sb->inode_start_address + (inode_id - 1) * sizeof(struct pseudo_inode), SEEK_SET);
}

/**
 * Function to set pointer in file to start of entered datablock
 * @param fs filesystem structure
 * @param datablock_id datablock identifier
 * @return information about operation success
 */
int set_file_datablock_position(file_system *fs, int32_t datablock_id) {
    return fseek(fs->file, fs->sb->data_start_address + (datablock_id - 1) * fs->sb->datablock_size, SEEK_SET);
}

/**
 * Function to translate inode datablock address to filesystem datablock address
 * @param fs filesystem structure
 * @param inode inode structure
 * @param datablock_id datablock inode-address
 * @return filesystem datablock identifier
 */
int32_t get_datablock_id(file_system *fs, struct pseudo_inode *inode, int32_t datablock_id) {
    int n = fs->sb->datablock_size / sizeof(int32_t);

    if(datablock_id < DIRECT_LINKS_COUNT) {
        return inode->direct[datablock_id];
    }
    datablock_id -= DIRECT_LINKS_COUNT;
    if(datablock_id <= n) {
        int32_t address;
        set_file_datablock_position(fs, inode->indirect[0]);
        fseek(fs->file, datablock_id * sizeof(int32_t), SEEK_CUR);
        fread(&address, sizeof(int32_t), 1, fs->file);
        return address;
    } else {
        datablock_id -= n;
        int p = datablock_id / n;
        datablock_id = datablock_id % n;
        int32_t address;
        set_file_datablock_position(fs, inode->indirect[1]);
        fseek(fs->file, p * sizeof(int32_t), SEEK_CUR);
        fread(&address, sizeof(int32_t), 1, fs->file);
        set_file_datablock_position(fs, address);
        fseek(fs->file, n * sizeof(int32_t), SEEK_CUR);
        fread(&address, sizeof(int32_t), 1, fs->file);
        return address;

    }
}

/**
 * Function to assign datablock to inode
 * @param fs filesystem structure
 * @param inode inode structure
 * @param datablock_id inode-address of datablock
 * @param datablock_address datablock identifier
 * @return information about operation success
 */
int set_datablock_id(file_system *fs, struct pseudo_inode *inode, int32_t datablock_id, int32_t datablock_address) {
    int n = fs->sb->datablock_size / sizeof(int32_t);
    if(datablock_id < DIRECT_LINKS_COUNT) {

        inode->direct[datablock_id] = datablock_address;
        return EXIT_SUCCESS;
    }
    datablock_id -= DIRECT_LINKS_COUNT;
    if(datablock_id <= n) {
        if(inode->indirect[0] == 0) {
            inode->indirect[0] = get_free_datablock_id(fs);
        }
        int32_t address;
        set_file_datablock_position(fs, inode->indirect[0]);
        fseek(fs->file, datablock_id * sizeof(int32_t), SEEK_CUR);
        fwrite(&datablock_address, sizeof(int32_t), 1, fs->file);
        return EXIT_SUCCESS;
    } else {
        if(inode->indirect[1] == 0) {
            inode->indirect[1] = get_free_datablock_id(fs);
        }
        datablock_id -= n;
        int p = datablock_id / n;
        datablock_id = datablock_id % n;
        int32_t address;
        set_file_datablock_position(fs, inode->indirect[1]);
        fseek(fs->file, p * sizeof(int32_t), SEEK_CUR);
        fread(&address, sizeof(int32_t), 1, fs->file);
        if(address == 0) {
            address = get_free_datablock_id(fs);
            fseek(fs->file, p * sizeof(int32_t), SEEK_CUR);
            fwrite(&address, sizeof(int32_t), 1, fs->file);
        }
        set_file_datablock_position(fs, address);
        fseek(fs->file, n * sizeof(int32_t), SEEK_CUR);
        fwrite(&datablock_address, sizeof(int32_t), 1, fs->file);
        return EXIT_SUCCESS;
    }
}

/**
 * Function to get number of inode from entered filesystem path
 * @param fs filesystem structure
 * @param parent inode identifier from which the path starts
 * @param path filesystem path
 * @param offset where we should stop finding
 * @return identifier to targeting inode
 */
int32_t get_inode_by_path(file_system *fs, int32_t parent, char *path, int offset) {
    int length = strlen(path);

    if(length == 0) {
        return parent;
    }
    int files_count = 1;

    int i;
    for(i = 0; i < length; i++) {
        if(path[i] == '/') {
            files_count++;
        }
    }

    if(path[length - 1] == '/' && length > 1) {
        files_count--;
    }

    if(path[0] == '/') {
        parent = 1;
        files_count--;
    }

    if(files_count == 1 && offset == 1) {
        return parent;
    }

    char *parent_name = NULL;
    char *folder = strtok(path, "/");
    int j = files_count;

    while(folder != NULL) {
        parent =  find_file_in_folder(fs, parent, folder);

        if(parent == 0) {
            //printf("Chyba! - %s není složka!\n", parent_name);
            return 0;
        }

        j--;
        if(j == offset) {
            return parent;
        }

        parent_name = folder;
        folder = strtok(NULL, "/");
    }
    return parent;
}

/**
 * Function to deallocate allocated inode
 * @param fs filesystem structure
 * @param inode_id inode identifier
 * @return information about operation success
 */
int free_inode(file_system *fs, int32_t inode_id) {
    char empty_block[sizeof(struct pseudo_inode)];
    memset(empty_block, 0, sizeof(struct pseudo_inode));
    set_file_inode_position(fs, inode_id);
    fwrite(empty_block, sizeof(struct pseudo_inode), 1, fs->file);

    char bitmap;
    fseek(fs->file, fs->sb->bitmapi_start_address + inode_id/8, SEEK_SET);
    fread(&bitmap, sizeof(char), 1, fs->file);
    u_char zero = 0;
    //bitmap &= zero << ((inode_id)%8);

    //printf("pred:  "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(bitmap));
    bitmap &= ~(1 << (8 - inode_id%8));
    //printf("po:  "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(bitmap));
    fseek(fs->file, fs->sb->bitmapi_start_address + inode_id/8, SEEK_SET);
    fwrite(&bitmap, sizeof(char), 1, fs->file);

    return EXIT_SUCCESS;

}

/**
 * Function to deallocate allocated datablock
 * @param fs filesystem structure
 * @param datablock_id datablock identifier
 * @return information about operation success
 */
int free_datablock(file_system *fs, int32_t datablock_id) {
    char empty_block[fs->sb->datablock_size];
    memset(empty_block, 0, fs->sb->datablock_size);
    set_file_datablock_position(fs, datablock_id);
    fwrite(empty_block, fs->sb->datablock_size, 1, fs->file);

    char bitmap;
    fseek(fs->file, fs->sb->bitmap_start_address + datablock_id/8, SEEK_SET);
    fread(&bitmap, sizeof(char), 1, fs->file);
    bitmap &= ~(1 << (8 - datablock_id%8));

    fseek(fs->file, fs->sb->bitmap_start_address + datablock_id/8, SEEK_SET);
    fwrite(&bitmap, sizeof(char), 1, fs->file);

    return EXIT_SUCCESS;
}

/**
 * Function to load inode structure from filesystem
 * @param fs filesystem structure
 * @param inode_id inode identifier
 * @param inode inode structure
 * @return information about operation success
 */
int load_inode(file_system *fs, int32_t inode_id, struct pseudo_inode *inode) {
    set_file_inode_position(fs, inode_id);
    fread(inode, sizeof(struct pseudo_inode), 1, fs->file);
    return EXIT_SUCCESS;
}

/**
 * Function to save inode structure to filesystem
 * @param fs filesystem structure
 * @param inode_id inode identifier
 * @param inode inode structure
 * @return information about operation success
 */
int save_inode(file_system *fs, int32_t inode_id, struct pseudo_inode *inode) {
    set_file_inode_position(fs, inode_id);
    fwrite(inode, sizeof(struct pseudo_inode), 1, fs->file);
    return EXIT_SUCCESS;
}