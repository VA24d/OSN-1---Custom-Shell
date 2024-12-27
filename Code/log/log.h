#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "../base/base.h"

#define LOG_FILE "/.command_history.log"
#define MAX_HISTORY 15
#define MAX_COMMAND_LENGTH 1024

// Function to initialize the log file
void initialize_log(char* homeDir);

// Function to get the total number of commands in the log file
// int get_log_size();

// Function to append a command to the log file
void log_command(const char *command, char* homeDir);

// Function to print the command log
void print_log(char* homeDir);

// Function to clear the log file
void purge_log(char* homeDir);

// Function to execute a command by index
void execute_log_command(int index, char *curDir, char *prevDir, char* homeDir);

void log_main(const char *arg, char *curDir, char *prevDir, char* homeDir);


#endif