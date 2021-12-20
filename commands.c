
#include "commands.h"

const int COMMANDS_COUNT = 15;
const cmd_handler COMMANDS[] = {
        {"cp", cp},
        {"mv", mv},
        {"rm", rm},
        {"mkdir", mkdir},
        {"rmdir", rmdir},
        {"ls", ls},
        {"cat", cat},
        {"cd", cd},
        {"pwd", pwd},
        {"info", info},
        {"incp", incp},
        {"outcp", outcp},
        {"load", load},
        {"format", format},
        {"ln", ln}
};

fcmd get_handler(char command[]) {
    int i;

    int command_length = strlen(command);

    for(i = 0; i < command_length; i++) {
        command[i] = tolower(command[i]);
    }

    for(i = 0; i < COMMANDS_COUNT; ++i) {
        if(!strcmp(COMMANDS[i].cmd, command)) {
            return COMMANDS[i].handler;
        }
    }

    return NULL;
}

int cp(file_system *fs, int argc, char **argv) {


    return EXIT_SUCCESS;
}

int mv(file_system *fs, int argc, char **argv) {


    return EXIT_SUCCESS;
}

int rm(file_system *fs, int argc, char **argv) {


    return EXIT_SUCCESS;
}

int mkdir(file_system *fs, int argc, char **argv) {
    if(argc < 1) {
        printf("Zadej parametr\n");
        return EXIT_FAILURE;
    }

    char *full_path = argv[0];
    int path_length = strlen(full_path);
    int i = path_length - 1;
    int j;

    if(full_path[path_length-1] == '/') {
        i--;
        path_length--;
    }

    for(j = i; j >= 0; j--) {
        if(full_path[j] == '/') {
            break;
        }
    }

    j++;

    char parent_path[j+1];
    strncpy(parent_path, full_path, j);
    parent_path[j] = '\0';

    char folder_name[path_length-j+1];
    strncpy(folder_name, &(full_path[j]), path_length-j);
    folder_name[path_length-j] = '\0';

    int32_t folder_node = get_inode_by_path(fs, fs->current_folder, parent_path);
    if(create_directory(fs, folder_node, folder_name) == EXIT_SUCCESS) {
        fflush(fs->file);
        printf("OK\n");
        return EXIT_SUCCESS;
    }
    //create_directory(fs, 1, "home");


    return EXIT_FAILURE;
}

int rmdir(file_system *fs, int argc, char **argv) {


    return EXIT_SUCCESS;
}

int ls(file_system *fs, int argc, char **argv) {
    int32_t folder;

    if(argc > 0) {
        folder = get_inode_by_path(fs, fs->current_folder, argv[0]);
    } else {
        folder = fs->current_folder;
    }
    print_folder_content(fs, folder);

    return EXIT_SUCCESS;
}

int cat(file_system *fs, int argc, char **argv) {

    if (argc < 1) {
        printf("PATH NOT EXISTS\n");
        return EXIT_FAILURE;
    }

    int file = get_inode_by_path(fs, fs->current_folder, argv[0]);

    if(file <= 0) {
        printf("FILE NOT FOUND\n");
        return EXIT_FAILURE;
    }

    struct pseudo_inode inode = { 0 };
    load_inode(fs, file, &inode);

    if(inode.isDirectory) {
        printf("FILE IS DIRECTORY\n");
        return EXIT_FAILURE;
    }

    int n = fs->sb->datablock_size / sizeof(int32_t);
    int k = DIRECT_LINKS_COUNT + n + n * n;
    int i;
    char buf[fs->sb->datablock_size + 1];
    buf[fs->sb->datablock_size] = '\0';

    long size = inode.file_size;
    long printed = 0;

    for(i = 0; i < k; i++) {
        long read;
        int datablock_id = get_datablock_id(fs, &inode, i);
        if(datablock_id == 0) break;

        if(size - printed >= fs->sb->datablock_size) {
            read = fs->sb->datablock_size;
        } else {
            read = size - printed;
        }
        memset(buf, 0, fs->sb->datablock_size);
        set_file_datablock_position(fs, datablock_id);
        fread(buf, read, 1, fs->file);
        buf[read] = '\0';
        printf("\n-------------------------------------------------\n");
        printf("%i %i\n", i, datablock_id);
        printf("\n-------------------------------------------------\n");
        printf("%s", buf);
    }
    printf("\nEND OF FILE");
    printf("\n");

    return EXIT_SUCCESS;
}

int cd(file_system *fs, int argc, char **argv) {
    int32_t parent;
    int counter;
    int i;
    linked_list *path;


    if(argc < 1) {
        printf("Zadej parametr!\n");
        return EXIT_FAILURE;
    }

    if(strlen(argv[0]) == 0) {
        return EXIT_SUCCESS;
    }


    if(argv[0][0] == '/') {
        parent = 1;
        path = linked_list_create();

    } else {
        parent = fs->current_folder;
        path = fs->path;
        if(!path) {
            printf("Chyba: Vnitrni chyba.");
            return EXIT_FAILURE;
        }
    }

    counter = 0;
    char *parent_name = NULL;
    char *folder = strtok(argv[0], "/");

    while(folder != NULL) {
        //parent = get_directory_item_inode(fs, parent, folder);
        parent = find_file_in_folder(fs, parent, folder);
        //printf("%s - %d\n", folder, parent);

        if(parent == 0) {
            printf("Chyba! - %s není složka!\n", parent_name);
            for(i = 0; i < counter; i++) {
                linked_list_remove_last(path);

            }

            return EXIT_FAILURE;
        } else {
            if(!strcmp(folder, ".")) {
            } else if(!strcmp(folder, "..")) {
                linked_list_remove_last(path);
                counter--;
            } else {
                linked_list_add(path, folder);
                counter++;
            }
        }

        parent_name = folder;
        folder = strtok(NULL, "/");
    }

    fs->current_folder = parent;
    if(fs->path != path) {
        linked_list_free(&fs->path);
        fs->path = path;
    }

    return EXIT_SUCCESS;
}

int pwd(file_system *fs, int argc, char **argv) {

    struct linked_list_item* item = fs->path->first;

    if(item == NULL) {
        printf("/");
    }

    while(item != NULL) {
        printf("/%s", item->name);
        item = item->next;
    }

    printf("\n");

    return EXIT_SUCCESS;
}

int info(file_system *fs, int argc, char **argv) {

    load_superblock(fs);

    printf("---------------- INFO -----------------\n");
    printf("Autor: %s\n", fs->sb->signature);
    printf("Description: %s\n", fs->sb->volume_descriptor);
    printf("---------------------------------------\n");


    return EXIT_SUCCESS;
}

int incp(file_system *fs, int argc, char **argv) {

    if(argc < 2) {
        printf("Nespravny pocet parametru\n");
        return EXIT_FAILURE;
    }

    char *path = argv[1];
    int path_length = strlen(path);
    char *file_name = "";
    int32_t current_directory = fs->current_folder;

    int pos = 0;
    if(path[path_length - 1] == '/') {
        path[path_length - 1 ] = '\0';
        file_name = strrchr(argv[0], '/');
        if(!file_name) {
            file_name = argv[0];
        }
    } else {
        file_name = strrchr(path, '/') + 1;
        path[path_length - strlen(file_name) - 1] = '\0';
    }

    printf("name: %s\n", file_name);
    printf("path: %s\n", path);

    int parent = get_inode_by_path(fs, current_directory, path);

    if(get_inode_by_path(fs, parent, file_name) > 0) {
        printf("FILE EXISTS\n");
        return EXIT_FAILURE;
    }

    printf("parent: %i\n", parent);



    create_file(fs, argv[0], parent, file_name);

    return EXIT_SUCCESS;
}

int outcp(file_system *fs, int argc, char **argv) {

    if(argc < 2) {
        return EXIT_FAILURE;
    }

    int file = get_inode_by_path(fs, fs->current_folder, argv[0]);
    printf("FILE: %i\n", file);
    struct pseudo_inode inode = { 0 };
    load_inode(fs, file, &inode);

    if((&inode)->isDirectory) {
        printf("SOUBOR JE SLOZKA\n");
        return EXIT_FAILURE;
    }

    printf("SIZE: %li\n", (&inode)->file_size);

    long size = (&inode)->file_size;

    FILE *dest_file = fopen(argv[1], "w");

    int n = fs->sb->datablock_size / sizeof(int32_t);
    int k = DIRECT_LINKS_COUNT + n + n * n;

    printf("PRVNI DATABLOCK: %i\n", (&inode)->direct[0]);

    char buf[fs->sb->datablock_size];
    int i = 0;
    printf("i: %i, k: %i, size: %li\n", i, k, size);
    for(i = 0; i < k && size > 0; i++) {
        int datablock_id = get_datablock_id(fs, &inode, i);
        printf("Datablock: %i\n", datablock_id);
        if(datablock_id == 0) {
            break;
        }

        printf("%i. cteni\n", i);

        set_file_datablock_position(fs, datablock_id);
        if(size <= fs->sb->datablock_size) {
            fread(buf, size, 1, fs->file);
            fwrite(buf, size, 1, dest_file);
        } else {
            fread(buf, fs->sb->datablock_size, 1, fs->file);
            fwrite(buf, fs->sb->datablock_size, 1, dest_file);
            size -= fs->sb->datablock_size;
        }

    }

    fclose(dest_file);

    return EXIT_SUCCESS;
}

int load(file_system *fs, int argc, char **argv) {


    return EXIT_SUCCESS;
}

int format(file_system *fs, int argc, char **argv) {
    printf("Formatuji :-)\n");

    return EXIT_SUCCESS;
}

int ln(file_system *fs, int argc, char **argv) {

    return EXIT_SUCCESS;
}
