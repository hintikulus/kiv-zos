#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "structures.h"
#include "filesystem.h"
#include "commands.h"
#include "inode.h"

#define COMMAND_QUIT "exit"

int main(int argc, char** argv) {

    if(argc <= 1) {
        printf("Zadej parametr: Název souboru\n");
        return EXIT_SUCCESS;
    }

    char *file_name = argv[1];

    printf("Hello there\n");
    char cmd[256] = { 0 };

    struct superblock sb;
    printf("%d\n", sizeof(sb));

    struct pseudo_inode inode;
    printf("%d\n", sizeof(inode));

    

    file_system fs = file_system_open(file_name);

    int size = 10 * 1024 * 1024;

    printf("Volam formatovani\n");
    file_system_format(&fs, size);
    printf("Ukončeno formatovani\n");


    while(1) {
        printf(">>> ");
        fgets(cmd, 256, stdin);
        int input_size = strlen(cmd);
        if(input_size <= 1) {
            continue;
        }
        cmd[input_size - 1] = '\000';

        int argc = 0;
        int i;

        for(i = 0; i < input_size; i++) {

            if(cmd[i] == ' ') {
                argc++;
            }
        }

        char* command = strtok(cmd, " ");

        char *argv[argc];
        
        for(i = 0; i < argc; i++) {
            argv[i] = strtok(NULL, " ");
        }

        printf("Ziskavam handler pro %s\n", command);

        fcmd handler = get_handler(command);

        if(handler) {
            printf("Mám handler\n");
            handler(argc, argv);
        } else {
            printf("Nemám handler\n");
        }

        printf("Příkaz: %s\n", cmd);
    }

    
    file_system_close(&fs);

    return EXIT_SUCCESS;
}