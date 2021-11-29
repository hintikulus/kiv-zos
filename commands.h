//
// Created by hintik on 28.11.21.
//

#ifndef SP_COMMANDS_H
#define SP_COMMANDS_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "inode.h"

typedef int(*fcmd)(file_system *fs, int, char **);

typedef struct _cmd_handler {
    char *cmd;
    fcmd handler;
} cmd_handler;


fcmd get_handler(char commands[]);

int cp(file_system *fs, int argc, char **argv);

int mv(file_system *fs, int argc, char **argv);

int rm(file_system *fs, int argc, char **argv);

int mkdir(file_system *fs, int argc, char **argv);

int rmdir(file_system *fs, int argc, char **argv);

int ls(file_system *fs, int argc, char **argv);

int cat(file_system *fs, int argc, char **argv);

int cd(file_system *fs, int argc, char **argv);

int pwd(file_system *fs, int argc, char **argv);

int info(file_system *fs, int argc, char **argv);

int incp(file_system *fs, int argc, char **argv);

int outcp(file_system *fs, int argc, char **argv);

int load(file_system *fs, int argc, char **argv);

int format(file_system *fs, int argc, char **argv);

int ln(file_system *fs, int argc, char **argv);

#endif //SP_COMMANDS_H
