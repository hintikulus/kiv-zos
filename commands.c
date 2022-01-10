#include "commands.h"

/**
 * Accessable cammand with their handle function
 */
const int COMMANDS_COUNT = 13;
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
        {"ln", ln},
};

/**
 * Function to obtaining handle function of specified command
 * @param command command
 * @return handle function
 */
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

/**
 * Function to handle copy (cp) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int cp(file_system *fs, int argc, char **argv) {

    if(argc < 2) {
        printf("MISSING OPERAND\n");
        return EXIT_SUCCESS;
    }

    char path[1024];
    strcpy(path, argv[0]);
    int src_inode = get_inode_by_path(fs, fs->current_folder, path, 0);
    if(src_inode == 0) {
        printf("FILE NOT FOUND\n");
        return EXIT_SUCCESS;
    }

    struct pseudo_inode inode_src = {};
    struct pseudo_inode inode_dest = {};
    load_inode(fs, src_inode, &inode_src);
    if((&inode_src)->isDirectory) {
        printf("FILE NOT FOUND\n");
        return EXIT_SUCCESS;
    }

    strcpy(path, argv[1]);
    int dest_inode = get_inode_by_path(fs, fs->current_folder, path, 0);
    if(dest_inode > 0) {
        printf("FILE EXISTS\n");
        return EXIT_SUCCESS;
    }

    strcpy(path, argv[1]);
    int dest_inode_parent = get_inode_by_path(fs, fs->current_folder, path, 1);

    char name[12] = "";
    int length = strlen(argv[1]);
    if(argv[1][length - 1] == '/') {
        strcpy(name, get_name_from_path(argv[0]));
    } else {
        strcpy(name, get_name_from_path(argv[1]));
    }

    int new_id = get_free_inode_id(fs);
    if(new_id == 0) {
        return EXIT_FAILURE;
    }

    (&inode_dest)->nodeid = new_id;
    (&inode_dest)->isDirectory = (&inode_src)->isDirectory;
    (&inode_dest)->file_size = (&inode_src)->file_size;
    (&inode_dest)->references = 1;

    int n = fs->sb->datablock_size / sizeof(int32_t);
    int k = DIRECT_LINKS_COUNT + n + n * n;
    int i;
    char buf[fs->sb->datablock_size + 1];
    buf[fs->sb->datablock_size] = '\0';

    long size = inode_src.file_size;
    long printed = 0;

    for(i = 0; i < k; i++) {
        long read;
        int datablock_id = get_datablock_id(fs, &inode_src, i);
        if(datablock_id == 0) break;

        if(size - printed >= fs->sb->datablock_size) {
            read = fs->sb->datablock_size;
        } else {
            read = size - printed;
        }
        memset(buf, 0, fs->sb->datablock_size);
        set_file_datablock_position(fs, datablock_id);
        fread(buf, read, 1, fs->file);

        int new_datablock = get_free_datablock_id(fs);
        if(new_datablock == 0) {
            return EXIT_FAILURE;
        }
        set_file_datablock_position(fs, new_datablock);
        fwrite(buf, fs->sb->datablock_size, 1, fs->file);
        set_datablock_id(fs, &inode_dest, i, new_datablock);

    }

    save_inode(fs, new_id, &inode_dest);
    set_directory_item(fs, dest_inode_parent, new_id, name);
    printf("OK\n");
    return EXIT_SUCCESS;
}

/**
 * Function to handle move (mv) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int mv(file_system *fs, int argc, char **argv) {

    if(argc < 2) {
        printf("MISSING OPERAND\n");
        return EXIT_SUCCESS;
    }

    char path[1024];
    strcpy(path, argv[0]);
    int src_inode = get_inode_by_path(fs, fs->current_folder, path, 0);
    if(src_inode == 0) {
        printf("FILE NOT FOUND\n");
        return EXIT_SUCCESS;
    }

    struct pseudo_inode inode = {};
    load_inode(fs, src_inode, &inode);
    if((&inode)->isDirectory) {
        printf("FILE IS A DIRECTORY\n");
        return EXIT_SUCCESS;
    }

    strcpy(path, argv[1]);
    int dest_inode = get_inode_by_path(fs, fs->current_folder, path, 0);
    if(dest_inode > 0) {
        printf("FILE EXISTS\n");
        return EXIT_SUCCESS;
    }

    strcpy(path, argv[0]);
    unset_directory_item(fs, path);

    strcpy(path, argv[1]);
    int dest_inode_parent = get_inode_by_path(fs, fs->current_folder, path, 1);

    char name[12] = "";
    int length = strlen(argv[1]);
    if(argv[1][length - 1] == '/') {
        strcpy(name, get_name_from_path(argv[0]));
    } else {
        strcpy(name, get_name_from_path(argv[1]));
    }
    set_directory_item(fs, dest_inode_parent, (&inode)->nodeid, name);

    printf("OK\n");
    return EXIT_SUCCESS;
}

/**
 * Function to handle remove (rm) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int rm(file_system *fs, int argc, char **argv) {

    if (argc < 1) {
        printf("MISSING OPERAND\n");
        return EXIT_FAILURE;
    }

    char path[1024] = "";
    strcpy(path, argv[0]);
    int file = get_inode_by_path(fs, fs->current_folder, argv[0], 0);

    if(file <= 0) {
        printf("FILE NOT FOUND\n");
        return EXIT_FAILURE;
    }

    struct pseudo_inode inode = { 0 };
    load_inode(fs, file, &inode);


    if((&inode)->isDirectory) {
        printf("FILE NOT FOUND\n");
        return EXIT_SUCCESS;
    }

    if((&inode)->references == 1) {
        int n = fs->sb->datablock_size / sizeof(int32_t);
        int k = DIRECT_LINKS_COUNT + n + n * n;
        int i;
        char buf[fs->sb->datablock_size + 1];
        buf[fs->sb->datablock_size] = '\0';

        long size = inode.file_size;
        long printed = 0;

        for (i = 0; i < k; i++) {
            int datablock_id = get_datablock_id(fs, &inode, i);
            if (datablock_id == 0) break;
            free_datablock(fs, datablock_id);
        }

        unset_directory_item(fs, path);
        free_inode(fs, (&inode)->nodeid);

    } else {
        unset_directory_item(fs, path);
        (&inode)->references--;
        save_inode(fs, (&inode)->nodeid, &inode);
    }

    printf("OK\n");
    return EXIT_SUCCESS;
}

/**
 * Function to handle copy (cp) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int mkdir(file_system *fs, int argc, char **argv) {
    if(argc < 1) {
        printf("MISSING OPERAND\n");
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

    int32_t folder_node = get_inode_by_path(fs, fs->current_folder, parent_path, 0);
    int return_value = create_directory(fs, folder_node, folder_name);

    if(return_value == 0) {
        fflush(fs->file);
        printf("OK\n");
        return EXIT_SUCCESS;
    } else if(return_value == 3) {
        printf("EXITS\n");
    } else {
        printf("PATH NOT FOUND\n");
    }
    return EXIT_FAILURE;
}

/**
 * Function to handle directory remove (rmdir) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int rmdir(file_system *fs, int argc, char **argv) {

    if(argc < 1) {
        printf("MISSING OPERAND\n");
        return EXIT_SUCCESS;
    }

    char path[1024];
    strcpy(path, argv[0]);

    int node_id = get_inode_by_path(fs, fs->current_folder, argv[0], 0);
    if(node_id == 0) {
        printf("FILE NOT FOUND\n");
        return EXIT_SUCCESS;
    }

    struct pseudo_inode inode = {};
    load_inode(fs, node_id, &inode);

    if(!(&inode)->isDirectory) {
        printf("FILE NOT FOUND\n");
        return EXIT_SUCCESS;
    }

    if(node_id == 1) {
        printf("YOU CANT REMOVE ROOT FOLDER\n");
        return EXIT_SUCCESS;
    }

    if((&inode)->references > 0) {
        printf("NOT EMPTY\n");
        return EXIT_SUCCESS;
    }

    char name[12] = "";
    strcpy(name, get_name_from_path(path));
    if(!strcmp(name, ".") || !strcmp(name, "..")) {
        printf("Nepřístupný argument\n");
        return EXIT_SUCCESS;
    }

    int count_db = (&inode)->file_size / fs->sb->datablock_size;
    int count = 0;

    int n = fs->sb->datablock_size / sizeof(int32_t);
    int k = DIRECT_LINKS_COUNT + n + n * n;
    int i;
    for(i = 0; i < k; i++) {
        int id = get_datablock_id(fs, &inode, i);
        if(id > 0) {
            count++;
            if (count == count_db) {
                break;
            }
        }
    }

    unset_directory_item(fs, path);
    free_inode(fs, node_id);
    printf("OK\n");
    return EXIT_SUCCESS;
}

/**
 * Function to handle list (ls) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int ls(file_system *fs, int argc, char **argv) {
    int32_t folder;

    if(argc > 0) {
        folder = get_inode_by_path(fs, fs->current_folder, argv[0], 0);
        if(folder <= 0) {
            printf("FILE NOT FOUND");
            return EXIT_SUCCESS;
        }
    } else {
        folder = fs->current_folder;
    }
    print_folder_content(fs, folder);

    return EXIT_SUCCESS;
}

/**
 * Function to handle print file (cat) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int cat(file_system *fs, int argc, char **argv) {

    if (argc < 1) {
        printf("MISSING OPERAND\n");
        return EXIT_FAILURE;
    }

    int file = get_inode_by_path(fs, fs->current_folder, argv[0], 0);

    if(file <= 0) {
        printf("FILE NOT FOUND\n");
        return EXIT_FAILURE;
    }

    struct pseudo_inode inode = { 0 };
    load_inode(fs, file, &inode);

    if(inode.isDirectory) {
        printf("FILE NOT FOUND\n");
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
        printf("%s", buf);
    }

    return EXIT_SUCCESS;
}

/**
 * Function to handle change directory (cd) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int cd(file_system *fs, int argc, char **argv) {
    int32_t parent;
    int counter;
    int i;
    linked_list *path;


    if(argc < 1) {
        printf("MISSING OPERAND\n");
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
            return EXIT_FAILURE;
        }
    }

    counter = 0;
    char *parent_name = NULL;
    char *folder = strtok(argv[0], "/");

    while(folder != NULL) {
        parent = find_file_in_folder(fs, parent, folder);
        if(parent == 0) {
            for(i = 0; i < counter; i++) {
                linked_list_remove_last(path);
                printf("FILE NOT FOUND\n");
            }

            return EXIT_SUCCESS;
        } else {
            struct pseudo_inode inode = {};
            load_inode(fs, parent, &inode);
            if(!inode.isDirectory) {
                for(i = 0; i < counter; i++) {
                    linked_list_remove_last(path);
                }
                printf("FILE NOT FOUND\n");
                return EXIT_SUCCESS;
            }

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

    printf("OK\n");
    return EXIT_SUCCESS;
}

/**
 * Function to handle print path (pwd) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
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

/**
 * Function to handle info (info) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int info(file_system *fs, int argc, char **argv) {

    if(argc < 1) {
        printf("MISSING OPERAND\n");
        return EXIT_FAILURE;
    }

    char path[1024];
    strcpy(path, argv[0]);
    int inode_number = get_inode_by_path(fs, fs->current_folder,path, 0);
    if(inode_number <= 0) {
        printf("FILE NOT FOUND\n");
        return EXIT_SUCCESS;
    }

    char name[12] = "";
    int depth = 0;
    size_t path_length = strlen(argv[0]);
    int i;

    if(argv[0][path_length - 1] == '/') {
        argv[0][path_length - 1] = '\0';
        path_length--;
    }

    if(path_length == 0) {

        strcpy(name, "/");
    } else {
        char *path_name = strrchr(argv[0], '/');
        if(!path_name) {
            strcpy(name, argv[0]);
        } else {
            path_name[0] = '\0';
            strcpy(name, path_name + sizeof(char));
        }

    }

    set_file_inode_position(fs, inode_number);
    struct pseudo_inode inode = {0};
    fread(&inode, sizeof(struct pseudo_inode), 1, fs->file);
    int count_db = ceil((&inode)->file_size / fs->sb->datablock_size);
    int count = 0;

    printf("%s - %ld - i-node %d -", name, (&inode)->file_size, (&inode)->nodeid);

    int n = fs->sb->datablock_size / sizeof(int32_t);
    int k = DIRECT_LINKS_COUNT + n + n * n;
    for(i = 0; i < k; i++) {
        int id = get_datablock_id(fs, &inode, i);
        //printf("%d \n", id);
        if(id > 0) {
            printf(" %d", id);
            if((&inode)->isDirectory) {
                count++;
                if(count == count_db) {
                    break;
                }
            }
        } else {
            if(!(&inode)->isDirectory) {
                break;
            }
        }
    }

    printf("\n");
    return EXIT_SUCCESS;
}

/**
 * Function to handle in filesystem copy (incp) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int incp(file_system *fs, int argc, char **argv) {

    if(argc < 2) {
        printf("MISSING OPERAND\n");
        return EXIT_FAILURE;
    }

    char *path = argv[1];
    size_t path_length = strlen(path);
    char file_name[12] = "";

    int32_t current_directory = fs->current_folder;

    int pos = 0;
    if(path[path_length - 1] == '/') {
        path[path_length - 1 ] = '\0';
        char *name = strrchr(argv[0], '/');
        if(!name) {
            strcpy(file_name, argv[0]);
        } else {
            strcpy(file_name, name);
        }
    } else {
        char *name = strrchr(path, '/');
        if(!name) {
            strcpy(file_name, path);
            path[0] = '\0';
        } else {
            strcpy(file_name, name + 1);
            name[0] = '\0';
        }
    }

    int parent = get_inode_by_path(fs, current_directory, path, 0);

    if(get_inode_by_path(fs, parent, file_name, 0) > 0) {
        printf("FILE EXISTS\n");
        return EXIT_FAILURE;
    }

    FILE *file = fopen(argv[0], "r");
    if(!file) {
        printf("PATH NOT FOUND\n");
        return EXIT_FAILURE;
    }

    if(create_file(fs, file, argv[0], parent, file_name) != 0) {
        printf("ERROR OCCURRED\n");
        return EXIT_FAILURE;
    }

    printf("OK\n");
    return EXIT_SUCCESS;
}

/**
 * Function to handle out filesystem copy (outcp) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int outcp(file_system *fs, int argc, char **argv) {

    if(argc < 2) {
        printf("MISSING OPERAND\n");
        return EXIT_FAILURE;
    }

    int file = get_inode_by_path(fs, fs->current_folder, argv[0], 0);
    if(file <= 0) {
        printf("FILE NOT FOUND\n");
    }
    struct pseudo_inode inode = { 0 };
    load_inode(fs, file, &inode);

    if((&inode)->isDirectory) {
        printf("FILE IS DIRECTORY\n");
        return EXIT_FAILURE;
    }

    long size = (&inode)->file_size;

    FILE *dest_file = fopen(argv[1], "w");
    if(!dest_file) {
        printf("PATH NOT FOUND\n");
        return EXIT_FAILURE;
    }

    int n = fs->sb->datablock_size / sizeof(int32_t);
    int k = DIRECT_LINKS_COUNT + n + n * n;

    char buf[fs->sb->datablock_size];
    int i = 0;
    for(i = 0; i < k && size > 0; i++) {
        int datablock_id = get_datablock_id(fs, &inode, i);
        if(datablock_id == 0) {
            break;
        }

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

    printf("OK\n");
    fclose(dest_file);

    return EXIT_SUCCESS;
}

/**
 * Function to handle hardlink (ln) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int ln(file_system *fs, int argc, char **argv) {

    if(argc < 2) {
        printf("MISSING OPERAND\n");
        return EXIT_FAILURE;
    }

    char name[12];

    char path[1024];
    strcpy(path, argv[1]);
    strcpy(name, get_name_from_path(argv[1]));

    int id = get_inode_by_path(fs, fs->current_folder, argv[1], 0);
    if(id > 0) {
        printf("SOUBOR JIZ EXISTUJE\n");
        return EXIT_SUCCESS;
    }

    int dest_id = get_inode_by_path(fs, fs->current_folder, argv[0], 0);

    if(dest_id <= 0) {
        printf("SOUBOR NEEXISTUJE\n");
        return EXIT_SUCCESS;
    }

    struct pseudo_inode inode_data = {};
    load_inode(fs, dest_id, &inode_data);
    if((&inode_data)->isDirectory) {
        printf("SOUBOR JE ADRESAR\n");
        return EXIT_SUCCESS;
    }

    int parent_id = get_inode_by_path(fs, fs->current_folder, path, 1);
    if(parent_id <= 0) {
        printf("NADRAZENA SLOZKA NEEXISTUJE\n");
        return EXIT_SUCCESS;
    }
    int length = strlen(argv[0]);

    (&inode_data)->references++;

    set_directory_item(fs, parent_id, dest_id, name);

    set_file_inode_position(fs, (&inode_data)->nodeid);
    fwrite(&inode_data, sizeof(struct pseudo_inode), 1, fs->file);

    //set_directory_item(fs, id, name);

    return EXIT_SUCCESS;
}
