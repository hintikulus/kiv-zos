
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

    if(full_path[path_length-1] == '/') {
        i--;
        path_length--;
    }

    for(i; i >= 0; i--) {
        if(full_path[i] == '/') {
            break;
        }
    }

    i++;

    printf("Po cyklu.\n");

    printf("i: %d\n", i);

    char parent_path[i+1];
    strncpy(parent_path, full_path, i);
    parent_path[i] = '\0';

    printf("co treba tady?\n");

    char folder_name[path_length-i];
    printf("%d\n", path_length-i);
    strncpy(folder_name, &(full_path[i]), path_length-i);
    folder_name[path_length-i] = '\0';

    if(folder_name[path_length-i-1] == '/') {
        folder_name[path_length-i-1] = '\0';
    }

    int n = strlen(folder_name) + 1;
    for(i = 0; i < n; i++) {
        printf("%d ", folder_name[i]);
    }
    printf("\n");

    //int path_inode = get_inode_by_path()

    int32_t folder_node = get_inode_by_path(fs, 1, parent_path);
    printf("Cesta odkazuje na inode: %d\n", folder_node);
    create_directory(fs, folder_node, folder_name);
    //create_directory(fs, 1, "home");

    printf("\n");

    printf("Vypis:\n");

    printf("Rodicovska slozka: %s\n", parent_path);
    printf("Nazev slozky: %s\n", folder_name);
    printf("Vytvoř slozku %s\n", argv[0]);


    return EXIT_SUCCESS;
}

int rmdir(file_system *fs, int argc, char **argv) {


    return EXIT_SUCCESS;
}

int ls(file_system *fs, int argc, char **argv) {


    return EXIT_SUCCESS;
}

int cat(file_system *fs, int argc, char **argv) {


    return EXIT_SUCCESS;
}

int cd(file_system *fs, int argc, char **argv) {


    return EXIT_SUCCESS;
}

int pwd(file_system *fs, int argc, char **argv) {


    return EXIT_SUCCESS;
}

int info(file_system *fs, int argc, char **argv) {


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
