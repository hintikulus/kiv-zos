
#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef int(*fcmd)(int, char **);

typedef struct _cmd_handler {
    char *cmd;
    fcmd handler;
} cmd_handler;


fcmd get_handler(char commands[]);

int cp(int argc, char **argv);

int mv(int argc, char **argv);

int rm(int argc, char **argv);

int mkdir(int argc, char **argv);

int rmdir(int argc, char **argv);

int ls(int argc, char **argv);

int cat(int argc, char **argv);

int cd(int argc, char **argv);

int pwd(int argc, char **argv);

int info(int argc, char **argv);

int incp(int argc, char **argv);

int outcp(int argc, char **argv);

int load(int argc, char **argv);

int format(int argc, char **argv);

int ln(int argc, char **argv);

#endif