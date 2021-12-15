#include "inode.h"

const int32_t ID_ITEM_FREE = 0;

void fill_default_sb(struct superblock *sb) {
    // add disk_size param
    strcpy(sb->signature, "== ext ==");
    strcpy(sb->volume_descriptor, "popis bla bla bla");
    sb->disk_size = 800; // 800B
    sb->datablock_size = 32; // takže max. počet "souborových" položek na cluster jsou dvě, protože sizeof(directory_item) = 16B
    sb->datablock_count = 10;
    sb->inode_count = 10;
    sb->bitmapi_start_address = sizeof(struct superblock); // konec sb
    sb->bitmap_start_address = sb->bitmapi_start_address + 10; // 80 bitů bitmapa
    sb->inode_start_address = sb->bitmap_start_address + 10;
    sb->data_start_address = sb->inode_start_address + 10 * sizeof(struct pseudo_inode);
}

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

    printf("Zbyle misto: %d\n", size);

}

int get_free_inode_id(file_system *fs) {
    if(!fs) {
        return 0;
    }

    int array_size = fs->sb->bitmap_start_address - fs->sb->bitmapi_start_address;
    u_char inode_bitmap[array_size];
    memset(inode_bitmap, 0, array_size * sizeof(u_char));

    fseek(fs->file, fs->sb->bitmapi_start_address, SEEK_SET);

    fread(inode_bitmap, sizeof(u_char) * fs->sb->bitmap_start_address - fs->sb->bitmapi_start_address, 1, fs->file);

    for (int i = 0 ; i < sizeof(inode_bitmap); i++) {
        for (int j=7; j>=0; j--) {
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

int get_free_datablock_id(file_system *fs) {

    u_char data_bitmap[fs->sb->inode_start_address - fs->sb->bitmap_start_address];
    memset(data_bitmap, '\0', sizeof(data_bitmap));
    fseek(fs->file, fs->sb->bitmap_start_address, SEEK_SET);
    fread(data_bitmap, sizeof(data_bitmap), 1, fs->file);

    for (int i = 0 ; i < sizeof(data_bitmap); i++) {
        for (int j=7; j>=0; j--) {
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

int create_inode(file_system *fs) {
    int id = get_free_inode_id(fs);

    if(id < 0) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

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
    (&inode)->references = 1;

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

int create_directory(file_system *fs, int32_t parent, char *name) {
    if(parent < 1) {
        printf("Neplatna cesta");
        return EXIT_FAILURE;
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
        printf("Zadany soubor neni slozka\n");
        return EXIT_FAILURE;
    }


    inode_id = find_file_in_folder(fs, parent, name);
    if(inode_id != 0) {
        printf("Soubor s takovym nazvem jiz existuje.\n");
        return EXIT_FAILURE;
    }

    free_directory_item = find_free_directory_item_in_folder(fs, parent, &free_directory_item_block);

    if(free_directory_item == 0 || free_directory_item_block == 0) {
        printf("Není místo");
        return EXIT_FAILURE;
    }

    /*
    for(i = 0; i < DIRECT_LINKS_COUNT; i++) {

       if((free_directory_item = find_free_directory_item_in_datablock(
          fs, (&parent_inode)->direct[i])) != 0) {
             free_directory_item_block = i;
             break;
       }

    } */

    inode_id = get_free_inode_id(fs);
    int32_t data_id = get_free_datablock_id(fs);

    if(inode_id == 0 || data_id == 0) {
        printf("Nebylo mozne ulozit soubor.\n");
        return EXIT_FAILURE;
    }

    struct pseudo_inode inode = {};
    (&inode)->isDirectory = true;
    (&inode)->nodeid = inode_id;
    (&inode)->references = 1;
    (&inode)->direct[0] = data_id;
    (&inode)->direct[1] = 0;
    (&inode)->direct[2] = 0;
    (&inode)->direct[3] = 0;
    (&inode)->direct[4] = 0;
    inode.indirect[0] = 0;
    inode.indirect[1] = 0;


    set_file_inode_position(fs, inode_id);
    fwrite(&inode, sizeof(inode), 1, fs->file);


    set_file_datablock_position(fs, data_id);
    struct directory_item di_root = {inode_id, "."};
    fwrite(&di_root, sizeof(struct directory_item), 1, fs->file);
    struct directory_item di_up = {parent, ".."};
    fwrite(&di_up, sizeof(struct directory_item), 1, fs->file);

    struct directory_item item = {};
    (&item)->inode = inode_id;
    strcpy((&item)->item_name, name);

    set_file_datablock_position(fs, free_directory_item_block);
    fseek(fs->file, free_directory_item * sizeof(struct directory_item), SEEK_CUR);
    fwrite(&item, sizeof(struct directory_item), 1, fs->file);

    return EXIT_SUCCESS;
}

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

                printf("To není možné\n");
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
                            printf("To není možné\n");
                            *datablock_number = link;
                            return found_id;
                        }
                    }
                }
            }

        }
    }

    printf("To není možné\n");
    return 0;
}

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

int create_datablock(file_system *fs) {
    int id = get_free_datablock_id(fs);

    if(id < 0) {
        return EXIT_FAILURE;
    }

    printf("Volný blok: %d\n", id);
    return EXIT_SUCCESS;
}

int set_directory_item(file_system *fs, int32_t parent, int32_t inode, char *name) {
    int datablock_number = 0;
    int free_directory_item = find_free_directory_item_in_folder(fs, parent, &datablock_number);

    if(free_directory_item == 0 || datablock_number == 0) {
        printf("Není místo");
        return EXIT_FAILURE;
    }
    printf("Nalezeno: %i v %i\n", free_directory_item, datablock_number);

    struct directory_item di = {};
    (&di)->inode = inode;
    strncpy((&di)->item_name, name, 11);

    set_file_datablock_position(fs, datablock_number);
    fseek(fs->file, free_directory_item * sizeof(struct directory_item), SEEK_CUR);
    fwrite(&di, sizeof(struct directory_item), 1, fs->file);


}

int set_file_inode_position(file_system *fs, int32_t inode_id) {
    if(!fs || !fs->sb) {
        return EXIT_FAILURE;
    }
    return fseek(fs->file, fs->sb->inode_start_address + (inode_id - 1) * sizeof(struct pseudo_inode), SEEK_SET);
}

int set_file_datablock_position(file_system *fs, int32_t datablock_id) {
    return fseek(fs->file, fs->sb->data_start_address + (datablock_id - 1) * fs->sb->datablock_size, SEEK_SET);
}

int32_t get_datablock_id(file_system *fs, struct pseudo_inode *inode, int32_t datablock_id) {
    int n = fs->sb->datablock_size / sizeof(int32_t);

    if(datablock_id <= DIRECT_LINKS_COUNT) {
        return inode->direct[datablock_id];
    }
    datablock_id -= DIRECT_LINKS_COUNT;
    if(datablock_id <= n) {
        int32_t address;
        set_file_datablock_position(fs, inode->indirect[0]);
        fseek(fs->file, datablock_id, SEEK_CUR);
        fread(&address, sizeof(int32_t), 1, fs->file);
        return address;
    } else {
        datablock_id -= n;
        int p = datablock_id / n;
        datablock_id = datablock_id % n;
        int32_t address;
        set_file_datablock_position(fs, inode->indirect[1]);
        fseek(fs->file, p, SEEK_CUR);
        fread(&address, sizeof(int32_t), 1, fs->file);
        set_file_datablock_position(fs, address);
        fseek(fs->file, n, SEEK_CUR);
        fread(&address, sizeof(int32_t), 1, fs->file);
        return address;

    }

    return -1;
}

int set_datablock_id(file_system *fs, struct pseudo_inode *inode, int32_t datablock_id, int32_t datablock_address) {
    int n = fs->sb->datablock_size / sizeof(int32_t);

    if(datablock_id <= DIRECT_LINKS_COUNT) {
        return inode->direct[datablock_id];
    }
    datablock_id -= DIRECT_LINKS_COUNT;
    if(datablock_id <= n) {
        int32_t address;
        set_file_datablock_position(fs, inode->indirect[0]);
        fseek(fs->file, datablock_id, SEEK_CUR);
        fwrite(&datablock_address, sizeof(int32_t), 1, fs->file);
        return EXIT_SUCCESS;
    } else {
        datablock_id -= n;
        int p = datablock_id / n;
        datablock_id = datablock_id % n;
        int32_t address;
        set_file_datablock_position(fs, inode->indirect[1]);
        fseek(fs->file, p, SEEK_CUR);
        fread(&address, sizeof(int32_t), 1, fs->file);
        set_file_datablock_position(fs, address);
        fseek(fs->file, n, SEEK_CUR);
        fwrite(&datablock_address, sizeof(int32_t), 1, fs->file);
        return EXIT_SUCCESS;
    }
}

int32_t get_inode_by_path(file_system *fs, int32_t parent, char *path) {

    if(strlen(path) == 0) {
        return parent;
    }

    int length = 0;
    int files_count = 1;

    while(path[length] != '\0') {
        if(path[length] == '/') files_count++;
        length++;
    }

    if(path[length - 1] == '/' && length > 1) {
        files_count--;
    }

    if(path[0] == '/') {
        parent = 1;
        files_count--;
    }


    char *parent_name = NULL;
    char *folder = strtok(path, "/");

    while(folder != NULL) {
        //parent = get_directory_item_inode(fs, parent, folder);
        parent =  find_file_in_folder(fs, parent, folder);

        //printf("%s - %d\n", folder, parent);
        if(parent == 0) {
            printf("Chyba! - %s není složka!\n", parent_name);
            return 0;
        }

        parent_name = folder;
        folder = strtok(NULL, "/");
    }

    return parent;
}

int free_inode(file_system *fs, int32_t inode_id) {
    char* empty_block = malloc(sizeof(struct pseudo_inode));
    memset(empty_block, 0, sizeof(struct pseudo_inode));
    set_file_inode_position(fs, inode_id);
    fwrite(empty_block, sizeof(struct pseudo_inode), 1, fs->file);

    char bitmap;
    fseek(fs->file, fs->sb->bitmapi_start_address + inode_id/8, SEEK_SET);
    fread(&bitmap, sizeof(char), 1, fs->file);
    u_char zero = 0;
    bitmap &= zero << (7 - inode_id%8);

    fseek(fs->file, fs->sb->bitmapi_start_address + inode_id/8, SEEK_SET);
    fwrite(&bitmap, sizeof(char), 1, fs->file);

    return EXIT_SUCCESS;

}

int free_datablock(file_system *fs, int32_t datablock_id) {
    char* empty_block = malloc(fs->sb->datablock_size);
    memset(empty_block, 0, fs->sb->datablock_size);
    set_file_datablock_position(fs, datablock_id);
    fwrite(empty_block, fs->sb->datablock_size, 1, fs->file);

    char bitmap;
    fseek(fs->file, fs->sb->bitmap_start_address + datablock_id/8, SEEK_SET);
    fread(&bitmap, sizeof(char), 1, fs->file);
    u_char zero = 0;
    bitmap &= zero << (7 - datablock_id%8);

    fseek(fs->file, fs->sb->bitmap_start_address + datablock_id/8, SEEK_SET);
    fwrite(&bitmap, sizeof(char), 1, fs->file);

    return EXIT_SUCCESS;
}