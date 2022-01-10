#include "filesystem.h"

/**
 * Function to open filesystem
 * @param file_name path to file with filesystem
 * @return filesystem structure
 */
file_system *file_system_open(char *file_name) {
    file_system *fs = NULL;

    if(!file_name) {
        return NULL;
    }

    FILE *file = fopen(file_name, "r+");
    if(!file) {
        return NULL;
    }

    fs = (file_system *) malloc(sizeof(file_system));

    if(!fs) {
        fclose(file);
        return NULL;
    }

    linked_list *path = linked_list_create();

    if(!path) {
        free(fs);
        fclose(file);
        return NULL;
    }

    struct superblock *sb = (struct superblock *) malloc(sizeof(struct superblock));

    if(!sb) {
        linked_list_free(&path);
        free(fs);
        fclose(file);
        return NULL;
    }

    fseek(file, 0, SEEK_SET);
    fread(sb, sizeof(struct superblock), 1, file);

    fs->current_folder = 1;
    fs->file_name = file_name;
    fs->file = file;
    fs->sb = sb;
    fs->path = path;
    return fs;
}

/**
 * Function to format filesystem to entered size
 * @param path path to file
 * @param size size of filesystem in bytes
 * @return filesystem structure
 */
file_system *file_system_format(char *path, int size) {
    int i;
    char zero[1] = { 0 };

    FILE *file = fopen(path, "wb+");

    if(!file) {
        return NULL;
    }

    zero[0] = '\0';
    fseek(file, 0, SEEK_SET);
    for(i = 0; i < size; i++) {
        fwrite(zero, sizeof(char), 1, file);
    }

    struct superblock sb = {};

    fill_superblock(&sb, size, DATABLOCK_SIZE);

    fseek(file, 0, SEEK_SET);
    write_superblock(file, &sb);

    int inode_bitmap_size = (&sb)->bitmap_start_address - (&sb)->bitmapi_start_address;
    int data_bitmap_size = (&sb)->inode_start_address - (&sb)->bitmap_start_address;

    u_char inode_bitmap[inode_bitmap_size];
    u_char data_bitmap[data_bitmap_size];

    memset(inode_bitmap, 0, inode_bitmap_size * sizeof(u_char));
    memset(data_bitmap, 0, data_bitmap_size * sizeof(u_char));

    fwrite(inode_bitmap, sizeof(u_char), inode_bitmap_size, file);
    fwrite(data_bitmap, sizeof(u_char), data_bitmap_size, file);

    fflush(file);
    fclose(file);

    file_system *fs = file_system_open(path);

    if(fs) {
        create_root_directory(fs);
    }
    return fs;
}

/**
 * Structure to write superblock to file
 * @param file opened file descriptor
 * @param sb pointer to superblock structure
 * @return
 */
int write_superblock(FILE *file, struct superblock *sb) {
    fseek(file, 0, SEEK_SET);
    fwrite(sb, sizeof(*sb), 1, file);
    return EXIT_SUCCESS;
}

/**
 * Function to close filesystem
 * @param fs filesystem structure
 * @return information about success of operation
 */
int file_system_close(file_system *fs) {
    free(fs->sb);
    linked_list_free(&fs->path);
    if(fs->file) {
        fclose(fs->file);
    }
    free(fs);
    return EXIT_SUCCESS;
}

/**
 * Function to print content of folder
 * @param fs filesystem structure
 * @param folder folder identifier
 * @return information about success
 */
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

    int n = fs->sb->datablock_size / sizeof(int32_t);
    int k = DIRECT_LINKS_COUNT + n + n * n;
    int counter = 0;
    int counter_db = ceil(inode.file_size / fs->sb->datablock_size);

    for(i = 0; i < k; i++) {
        int id = get_datablock_id(fs, &inode, i);
        if(id != 0) {
            set_file_datablock_position(fs, id);
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
            counter++;

            if(counter == counter_db) {
                break;
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * Function to fill datablock with data
 * @param fs filesystem structure
 * @param data data array
 * @param size data size
 * @return information about operation success
 */
int fill_datablock(file_system *fs, char *data, long size) {
    int id = get_free_datablock_id(fs);

    if(id == 0) {
        return 0;
    }

    set_file_datablock_position(fs, id);
    fwrite(data, size, 1, fs->file);
    fflush(fs->file);

    return id;
}

/**
 * Function to create file in filesystem from existing file
 * @param fs filesystem structure
 * @param source_file source file
 * @param path1 path to file
 * @param parent parent folder of the new file
 * @param name name of creating file
 * @return information of the operation success
 */
int create_file(file_system *fs, FILE *source_file, char *path1, int parent, char *name) {
    if(!fs || !path1 || !name) {
        return 0;
    }
    char buf[fs->sb->datablock_size];

    if(!source_file) {
        return 0;
    }

    int inode_id = get_free_inode_id(fs);

    if(inode_id == 0) {
        return 0;
    }

    struct pseudo_inode inode = {};
    (&inode)->isDirectory = false;
    (&inode)->nodeid = inode_id;
    (&inode)->references = 1;

    fseek(source_file, 0, SEEK_END);
    long size = ftell(source_file);
    fseek(source_file, 0, SEEK_SET);

    long copied_size = 0;
    int i;

    int n = fs->sb->datablock_size / sizeof(int32_t);
    int k = DIRECT_LINKS_COUNT + n + n * n;

    for(i = 0; i < k && copied_size < size; i++) {
        long read_size;

        if(size - copied_size >= fs->sb->datablock_size) {
            read_size = fs->sb->datablock_size;
        } else {
            read_size = size - copied_size;
        }

        fread(buf, read_size, 1, source_file);
        int datablock = fill_datablock(fs, buf, read_size);
        if (datablock == 0) {
            //Chyba
            break;
        }
        set_datablock_id(fs, &inode, i, datablock);
        int id = get_datablock_id(fs, &inode, i);
        copied_size += fs->sb->datablock_size;
    }

    (&inode)->file_size = size;

    set_file_inode_position(fs, inode_id);
    fwrite(&inode, sizeof(struct pseudo_inode), 1, fs->file);

    set_directory_item(fs, parent, inode_id, name);
    fclose(source_file);

    fflush(fs->file);
}

/**
 * Function to get filename from entered path
 * @param path filesystem path
 * @return pointer to position on path where starts file name
 */
char *get_name_from_path(char *path) {

    int length = strlen(path);
    if(path[length - 1] == '/') {
        path[length - 1] = '\0';
    }

    char *name = strrchr(path, '/');
    if(name) {
        return name + 1;
    } else {
        return path;
    }


}

