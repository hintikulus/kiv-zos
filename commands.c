
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

int cp(int argc, char **argv) {


}

int mv(int argc, char **argv) {


}

int rm(int argc, char **argv) {


}

int mkdir(int argc, char **argv) {
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

    char parent_path[i+1];
    strncpy(parent_path, full_path, i);
    parent_path[i] = '\0';

    char folder_name[path_length-i];
    strncpy(folder_name, &(full_path[i+1]), path_length-i);
    ///folder_name[path_length-i+1] = '\0';

    if(folder_name[path_length-i-1] == '/') {
        folder_name[path_length-i-1] = '\0';
    }

    int n = strlen(folder_name) + 1;
    for(i = 0; i < n; i++) {
        printf("%d ", folder_name[i]);
    }

    //int path_inode = get_inode_by_path()

    printf("\n");

    printf("Rodicovska slozka: %s\n", parent_path);
    printf("Nazev slozky: %s\n", folder_name);
    printf("Vytvoř slozku %s\n", argv[0]);



}

int rmdir(int argc, char **argv) {


}

int ls(int argc, char **argv) {


}

int cat(int argc, char **argv) {


}

int cd(int argc, char **argv) {


}

int pwd(int argc, char **argv) {


}

int info(int argc, char **argv) {


}

int incp(int argc, char **argv) {


}

int outcp(int argc, char **argv) {


}

int load(int argc, char **argv) {


}

int format(int argc, char **argv) {
    printf("Formatuji :-)\n");

    return EXIT_SUCCESS;
}

int ln(int argc, char **argv) {

    return EXIT_SUCCESS;
}
