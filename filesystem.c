#include "filesystem.h"

file_system *file_system_open(char *file_name) {
    file_system *FS = NULL;

    if(!file_name) {
        return NULL;
    }

    FS = (file_system *) malloc(sizeof(file_system));

    if(!FS) {
        return NULL;
    }

    //{ file_name, fopen(file_name, "wb+") };

    linked_list *path = linked_list_create();

    if(!path) {
        free(FS);
        return NULL;
    }

    //memcpy(sb, &sb_str, sizeof(struct superblock));

    //if(!sb) {
    //  free(FS);
    //  return NULL;
    //}

    FILE *file = fopen(file_name, "wb+");

    if(!file) {
        linked_list_free(&path);
    }

    FS->current_folder = 1;
    FS->file_name = file_name;
    FS->file = file;
    FS->sb = NULL;
    FS->path = path;

    return FS;
}

int file_system_format(file_system *fs, int size) {
    int i;
    char nula[1] = { 0 };

    if(!fs || !fs->file) {
        return EXIT_FAILURE;
    }

    //nula = (char *) malloc(sizeof(char));
    //nula[0] = '\0';

    //struct superblock sb = {};
    //memset(sb, '\0', sizeof(struct superblock));

    printf("Formatuji %ld\n", sizeof(*nula));

    //struct superblock sb = {};
    //fill_default_sb(fs->sb);

    nula[0] = '\0';
    fseek(fs->file, 0, SEEK_SET);
    for(i = 0; i < size; i++) {
        fwrite(nula, sizeof(char), 1, fs->file);
    }

    //free(nula);

    printf("Naplňuji superblock: \n");
    struct superblock *sb = (struct superblock *) malloc(sizeof(struct superblock));

    if(sb == NULL) {
        return EXIT_FAILURE;
    }

    fill_default_sb(sb);
    fill_superblock(sb, size, 4*1024);
    fs->sb = sb;

    printf("Výsledky formátování: \n");
    printf("Počet inodů: %d\n", fs->sb->inode_count);
    printf("Počet dat.blocků: %d\n", fs->sb->datablock_count);

    //fs->sb = &sb;
    write_superblock(fs, fs->sb);


    int inode_bitmap_size = (fs->sb)->bitmap_start_address - (fs->sb)->bitmapi_start_address;
    int data_bitmap_size = (fs->sb)->inode_start_address - (fs->sb)->bitmap_start_address;

    u_char inode_bitmap[inode_bitmap_size];
    u_char data_bitmap[data_bitmap_size];

    memset(inode_bitmap, 0, inode_bitmap_size * sizeof(u_char));
    memset(data_bitmap, 0, data_bitmap_size * sizeof(u_char));

    printf("Velikost: %ld\n", sizeof(inode_bitmap));

    printf("zapisuji bitmapy\n");
    fwrite(inode_bitmap, sizeof(u_char), inode_bitmap_size, fs->file);
    fwrite(data_bitmap, sizeof(u_char), data_bitmap_size, fs->file);
    //u_char inode_bitmap[inode_bitmap_size] = { 0 };
    //u_char data_bitmap[data_bitmap_size] = { 0 };
    printf("konec zapisovani bitmap\n");

    printf("halo?\n");
    create_root_directory(fs);



    printf("Tak tady?\n");
    create_directory(fs, 1, "homes");
    create_directory(fs, 1, "etc");
    create_directory(fs, 1, "var");
    create_directory(fs, 2, "hintik");
    create_directory(fs, 2, "petr");
    create_directory(fs, 6, "videa");
    create_directory(fs, 2, "hintik");

    fs->current_folder = 1;

    //get_inode_by_path(fs, 1, "/file/obrazky/dovolena/");
    //get_inode_by_path(fs, 1, "file/obrazky/dovolena/");
    //get_inode_by_path(fs, 1, "/file/obrazky/dovolena");
    //get_inode_by_path(fs, 1, "file/obrazky/dovolena");

    fflush(fs->file);

    printf("Velikost directory_item: %d\n", sizeof(struct directory_item));
    printf("Velikost dat. bloku: %d\n", fs->sb->datablock_size);
    printf("Velikost pocet itemů v dat. bloku: %d\n", fs->sb->datablock_size/sizeof(struct directory_item));

    return EXIT_SUCCESS;
}

int file_system_create(FILE file, int size, int block_size) {

    return EXIT_SUCCESS;
}


int write_superblock(file_system *fs, struct superblock *sb) {

    fseek(fs->file, 0, SEEK_SET);

    printf("Size:%ld\n", sizeof(*sb));
    fwrite(sb, sizeof(*sb), 1, fs->file);

    return EXIT_SUCCESS;
}

int load_superblock(file_system *fs) {
    fseek(fs->file, 0, SEEK_SET);

    fread(fs->sb, sizeof(struct superblock), 1, fs->file);
    return EXIT_SUCCESS;
}

int file_system_close(file_system *fs) {
    free(fs->sb);
    linked_list_free(&fs->path);
    fclose(fs->file);
    free(fs);
    return EXIT_SUCCESS;
}

int print_folder_content(file_system *fs, int32_t folder) {
    struct pseudo_inode inode, file_inode;
    int i, j;
    int item_count = fs->sb->datablock_size/sizeof(struct directory_item);
    struct directory_item items[item_count];

    set_file_inode_position(fs, folder);
    fread(&inode, sizeof(struct pseudo_inode), 1, fs->file);

    if(!inode.isDirectory) {
        printf("PATH NOT FOUND\n");
        return EXIT_FAILURE;
    }

    for(i = 0; i < DIRECT_LINKS_COUNT; i++) {
        if(inode.direct[i] == 0) {
            continue;
        }

        set_file_datablock_position(fs, inode.direct[i]);
        fread(items, sizeof(items), 1, fs->file);

        for(j = 0; j < item_count; j++) {
            if(items[j].inode != 0) {
                set_file_inode_position(fs, items[j].inode);
                fread(&file_inode, sizeof(struct pseudo_inode), 1, fs->file);

                if(file_inode.isDirectory) {
                    printf("+ %s\n", (&items[j])->item_name);
                } else {
                    printf("- %s\n", (&items[j])->item_name);
                }
            }
        }

    }

    return EXIT_SUCCESS;
}



int create_file(file_system *fs, char *path1, char *path2) {
    if(!fs || !path1 || !path2) {
        return 0;
    }

    FILE *source_file = fopen(path1, "r");
    char *buf[fs->sb->datablock_size];

    if(!source_file) {
        return 0;
    }




    fclose(source_file);
}