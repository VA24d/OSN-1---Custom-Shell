#ifndef REVEAL_H
#define REVEAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <errno.h>

#define PATH_MAX 4096

// Function to check if a file is hidden
int is_hidden(const char *name);

// Function to print file or directory details with color coding
void print_file_info(const char *path, const char *name, int show_extra);

// Function to list files in a directory with flags
void list_directory(const char *path, int show_all, int show_extra);

void reveal(char *arg, char* prevDir);

#endif