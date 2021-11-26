#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "structures.h"
#include "filesystem.h"
#include "commands.h"
#include "inode.h"
#include "linkedlist.h"

#define COMMAND_QUIT "exit"

int main(int argc, char** argv) {
    file_system *fs;
    linked_list *list;
    char *file_name;
    int size;
    struct superblock sb;
    struct pseudo_inode inode;
    char cmd[256] = { 0 };
    struct linked_list_item *item;

    if(argc <= 1) {
        printf("Zadej parametr: Název souboru\n");
        return EXIT_SUCCESS;
    }

    file_name = argv[1];

    printf("Hello there\n");

    printf("%d\n", sizeof(sb));

    printf("%d\n", sizeof(inode));

    

    fs = file_system_open(file_name);

    size = 10 * 1024 * 1024;

    printf("Volam formatovani\n");
    file_system_format(fs, size);
    printf("Ukončeno formatovani\n");

    printf("##### %d ##### (2)\n", (fs)->sb->inode_start_address);
    list = linked_list_create();

    linked_list_add(list, "text");
    linked_list_add(list, "slozka");
    linked_list_add(list, "dalsi_slozka");
    linked_list_remove_last(list);
    linked_list_add(list, "proboha");
    linked_list_remove_last(list);

    printf("Velikost listu: %d\n", list->size);
    printf("Prvni: %s\n", list->first->name);

    item = list->first;

    while(item->next) {
        printf("%s/", item->name);
        item = item->next;
    }
    printf("%s\n", item->name);
    
    linked_list_free(&list);

    while(1) {
        char *command;
        int input_size;
        int argc;
        int i;
        fcmd handler;
        char **argv;

        printf(">>> ");
        fgets(cmd, 256, stdin);
        input_size = strlen(cmd);
        if(input_size <= 1) {
            continue;
        }
        cmd[input_size - 1] = '\000';

        argc = 0;

        for(i = 0; i < input_size; i++) {

            if(cmd[i] == ' ') {
                argc++;
            }
        }

        command = strtok(cmd, " ");

        argv = (char **) malloc(sizeof(char *) * argc);
        if(!argv) {
            continue;
        }

        
        for(i = 0; i < argc; i++) {
            argv[i] = strtok(NULL, " ");
        }

        printf("Ziskavam handler pro %s\n", command);

        handler = get_handler(command);

        if(handler) {
            printf("Mám handler\n");
            handler(fs, argc, argv);
        } else {
            printf("Nemám handler\n");
        }

        printf("Příkaz: %s\n", cmd);

        if(!strcmp(cmd, "exit")) {
            break;
        }
    }

    
    file_system_close(fs);

    return EXIT_SUCCESS;
}