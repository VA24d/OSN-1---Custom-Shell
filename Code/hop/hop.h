#ifndef HOP_H
#define HOP_H

#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include "../base/base.h"

#define true 1
// #define in_size 4096
#define MAX_COMMANDS 100
#define MAX_COMMAND_LENGTH 1024
// #define PATH_MAX 4096


void hop_main(char *arg, char *curDir, char *prevDir, char *homeDir);

#endif