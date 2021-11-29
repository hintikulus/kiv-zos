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
    int size;
    char cmd[256] = { 0 };
    char cmd_temp[256] = { 0 };
    struct linked_list_item *item;

    if(argc <= 1) {
        printf("Zadej parametr: Název souboru\n");
        return EXIT_SUCCESS;
    }

    fs = file_system_open(argv[1]);

    size = 1 * 1024 * 1024;

    printf("Volam formatovani\n");
    file_system_format(fs, size);
    printf("Ukončeno formatovani\n");

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
        char *command, *arg, *cmd2;
        int input_size;
        int argc;
        int i;
        fcmd handler;
        char **argv;

        struct linked_list_item* item = fs->path->first;
        printf("(%d) ", fs->current_folder);

        if(item == NULL) {
            printf("/");
        }

        while(item != NULL) {
            printf("/%s", item->name);
            item = item->next;
        }

        printf("$ ");
        fgets(cmd, 256, stdin);
        input_size = strlen(cmd);

        if(input_size <= 1) {
            continue;
        }
        cmd[input_size - 1] = '\000';

        strcpy(cmd_temp, cmd);
        char *counter = cmd;

        strtok(cmd_temp, " ");
        if(!argv) {
            continue;
        }

        //argc = 0;
        arg = strtok(NULL, " ");
        argc = 0;

        while(arg != NULL) {
            argc++;
            arg = strtok(NULL, " ");
        }

        argv = (char **) malloc(sizeof(char *) * argc);
        if(!argv) {
            continue;
        }

        command = strtok(cmd, " ");
        arg = strtok(NULL, " ");

        i = 0;
        while(arg != NULL) {
            argv[i] = arg;
            i++;
            arg = strtok(NULL, " ");
        }

        if(!strcmp(cmd, "exit")) {
            free(argv);
            break;
        }

        handler = get_handler(command);

        if(handler) {
            handler(fs, argc, argv);
        } else {
            printf("Neznámý příkaz.\n");
        }
        free(argv);
    }


    file_system_close(fs);

    return EXIT_SUCCESS;
}