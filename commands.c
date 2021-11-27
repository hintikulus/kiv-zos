
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


    return EXIT_SUCCESS;
}

int outcp(file_system *fs, int argc, char **argv) {


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
