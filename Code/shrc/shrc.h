#ifndef SHRC_H
#define SHRC_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_ALIASES 100
#define MAX_FUNCTIONS 100
#define MAX_COMMAND_LENGTH 1024
#define MAX_BODY_LINES 100

typedef struct {
    char alias_name[256];
    char command[1024];
} Alias;

typedef struct {
    char func_name[256];
    char body[MAX_BODY_LINES][MAX_COMMAND_LENGTH];
    char line[MAX_COMMAND_LENGTH]; // To store the concatenated function body
    int num_lines;
} Function;

extern int alias_count;
extern int func_count;

extern Alias aliases[MAX_ALIASES];
extern Function functions[MAX_FUNCTIONS];

void load_myshrc(char *homeDir);

void print_loadedSHRC();

#endif // SHRC_H
