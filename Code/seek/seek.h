#ifndef SEEK_H
#define SEEK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>


// void parse_input(char *input, char *flags, char *search, char *target_directory);
// void seek(const char *flags, const char *search, const char *target_directory, char *prevDir, char *homeDir);
void main_seek(char *input, char *prevDir, char *homeDir);

#endif
