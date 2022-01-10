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

/**
 * typedef of handle function
 */
typedef int(*fcmd)(file_system *fs, int, char **);

/**
 * structure of command handler
 */
typedef struct _cmd_handler {
    char *cmd;
    fcmd handler;
} cmd_handler;

/**
 * Function to obtaining handle function of specified command
 * @param command command
 * @return handle function
 */
fcmd get_handler(char commands[]);

/**
 * Function to handle copy (cp) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int cp(file_system *fs, int argc, char **argv);

/**
 * Function to handle move (mv) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int mv(file_system *fs, int argc, char **argv);

/**
 * Function to handle remove (rm) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int rm(file_system *fs, int argc, char **argv);

/**
 * Function to handle copy (cp) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int mkdir(file_system *fs, int argc, char **argv);

/**
 * Function to handle directory remove (rmdir) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int rmdir(file_system *fs, int argc, char **argv);

/**
 * Function to handle list (ls) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int ls(file_system *fs, int argc, char **argv);

/**
 * Function to handle print file (cat) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int cat(file_system *fs, int argc, char **argv);

/**
 * Function to handle change directory (cd) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int cd(file_system *fs, int argc, char **argv);

/**
 * Function to handle print path (pwd) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int pwd(file_system *fs, int argc, char **argv);

/**
 * Function to handle info (info) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int info(file_system *fs, int argc, char **argv);

/**
 * Function to handle in filesystem copy (incp) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int incp(file_system *fs, int argc, char **argv);

/**
 * Function to handle out filesystem copy (outcp) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int outcp(file_system *fs, int argc, char **argv);

/**
 * Function to handle hardlink (ln) command
 * @param fs structure of filesystem
 * @param argc number of arguments given with the command
 * @param argv arguments given with the command
 * @return information about success of the function
 */
int ln(file_system *fs, int argc, char **argv);
#endif //SP_COMMANDS_H
