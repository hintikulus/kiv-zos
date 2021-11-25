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

    linked_list *list = linked_list_create();
    char *text = "Nazev";
    linked_list_add(list, "text");
    linked_list_add(list, "slozka");
    linked_list_add(list, "dalsi_slozka");
    linked_list_remove_last(list);
    linked_list_add(list, "proboha");
    linked_list_remove_last(list);

    printf("Velikost listu: %d\n", list->size);
    printf("Prvni: %s\n", list->first->name);

    struct linked_list_item *item = list->first;

    while(item->next) {
        printf("%s/", item->name);
        item = item->next;
    }
    printf("%s\n", item->name);
    
    linked_list_free(&list);

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

        if(!strcmp(cmd, "exit")) {
            break;
        }
    }

    
    file_system_close(&fs);

    return EXIT_SUCCESS;
}