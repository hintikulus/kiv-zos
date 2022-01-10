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

/**
 * Function proccess one line command
 * @param fs structure of filesystem
 * @param cmd command string
 * @param path path to the file
 * @return information about success of the function
 */
int process_line(file_system **fs, char *cmd, char *path) {
    char cmd_temp[256] = { 0 };
    int input_size = strlen(cmd);
    char *command, *arg;
    int arg_c;
    int i;
    fcmd handler;
    char **arg_v;

    if(input_size <= 1) {
        return EXIT_SUCCESS;
    }

    /**
     * Removing new line character
     */
    if(cmd[input_size - 1] == '\n') {
        cmd[input_size - 1] = '\000';
    }

    /**
     * Spliting arguments to char pointer array
     */
    strcpy(cmd_temp, cmd);
    char *counter = cmd;

    strtok(cmd_temp, " ");

    //arg_c = 0;
    arg = strtok(NULL, " ");
    arg_c = 0;

    while(arg != NULL) {
        arg_c++;
        arg = strtok(NULL, " ");
    }

    arg_v = (char **) malloc(sizeof(char *) * arg_c);
    if(!arg_v) {
        return EXIT_FAILURE;
    }

    command = strtok(cmd, " ");
    arg = strtok(NULL, " ");

    i = 0;
    while(arg != NULL) {
        arg_v[i] = arg;
        i++;
        arg = strtok(NULL, " ");
    }

    /**
     * Handling non-filesystem commands
     */
    if(!strcmp(cmd, COMMAND_QUIT)) {
        file_system_close(*fs);
        free(arg_v);
        exit(EXIT_SUCCESS);
    }

    if(!strcmp(cmd, "format")) {
        if(arg_c < 1) {
            printf("MISSING SIZE PARAMETER\n");

            free(arg_v);
            return EXIT_SUCCESS;
        }

        int minimum = sizeof(struct pseudo_inode) + 2 + sizeof(struct superblock) + DATABLOCK_SIZE;
        int formated_size = transfer_size(arg_v[0]);

        if(formated_size < minimum) {
            free(arg_v);
            printf("CANNOT CREATE FILE - TOO SMALL SIZE\n");
            return EXIT_SUCCESS;
        }

        if(*fs) {
            file_system_close(*fs);
        }

        *fs = file_system_format(path, formated_size);
        if(!*fs) {
            free(arg_v);
            return EXIT_SUCCESS;
        }

        if(!(*fs)->file) {
            printf("CANNOT CREATE FILE\n");
            free(arg_v);
            return EXIT_FAILURE;
        }

        if(!(*fs)->file) {
            printf("CANNOT CREATE FILE\n");
        }

        printf("OK\n");
        free(arg_v);
        return EXIT_FAILURE;
    }

    if(!strcmp(cmd, "load")) {
        if(arg_c < 1) {
            free(arg_v);
            return EXIT_SUCCESS;
        }
        FILE  *f = fopen(arg_v[0], "r");
        if(!f) {
            printf("FILE NOT FOUND\n");
            free(arg_v);
            return EXIT_SUCCESS;
        }

        char *line = NULL;
        size_t len = 0;
        ssize_t read;
        while((read = getline(&line, &len, f)) != -1) {
            process_line(fs, line, path);
        }

        printf("OK\n");

        free(line);
        fclose(f);
        free(arg_v);
        return EXIT_SUCCESS;
    }

    if(*fs && (*fs)->file) {
        handler = get_handler(command);

        if (handler) {
            handler(*fs, arg_c, arg_v);
            fflush((*fs)->file);
        } else {
            printf("COMMAND NOT FOUND\n");
        }
    } else {
        printf("FORMAT FILESYSTEM FIRST\n");
    }

    free(arg_v);
    return EXIT_SUCCESS;
}

/**
 * Program entry point with endless loop for command input.
 * @param argc number of program arguments
 * @param argv values of program arguments
 * @return application exit status
1 */
int main(int argc, char** argv) {
    file_system *fs;
    linked_list *list;
    int size;
    char cmd[256] = { 0 };
    char cmd_temp[256] = { 0 };
    struct linked_list_item *item;

    if(argc <= 1) {
        printf("MISSING ARGUMENT: FILENAME\n");
        return EXIT_SUCCESS;
    }

    fs = file_system_open(argv[1]);


    while(1) {
        char *command, *arg;
        int input_size;
        int arg_c;
        int i;
        fcmd handler;
        char **arg_v;

        if (fs) {
            struct linked_list_item *item = fs->path->first;

            if (item == NULL) {
                printf("/");
            }

            while (item != NULL) {
                printf("/%s", item->name);
                item = item->next;
            }

            printf("$ ");

        } else {
            printf("# ");
        }

        fgets(cmd, 256, stdin);
        process_line(&fs, cmd, argv[1]);
        continue;
    }

}

/**
 * Function to convert entered string value to byte size
 * @param size string size value
 * @return size in bytes
 */
int transfer_size(char *size) {
    char size1[256] = "";
    strcpy(size1, size);

    int value = 0;
    char unit[256] = "";
    int unit_value = 1;

    sscanf(size1, "%d%s", &value, unit);

    if(!strcmp(unit, "B")) unit_value = 1;
    if(!strcmp(unit, "kB")) unit_value = 1000;
    if(!strcmp(unit, "kiB")) unit_value = 1024;
    if(!strcmp(unit, "MB")) unit_value = 1000 * 1000;
    if(!strcmp(unit, "MiB")) unit_value = 1024 * 1024;
    if(!strcmp(unit, "GB")) unit_value = 1000 * 1000 * 1000;
    if(!strcmp(unit, "GiB")) unit_value = 1024 * 1024 * 1024;

    int result = value * unit_value;

    return result;
}