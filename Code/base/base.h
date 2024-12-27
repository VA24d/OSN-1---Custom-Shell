#ifndef CORE_H
#define CORE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../hop/hop.h"
#include "../reveal/reveal.h"
#include "../log/log.h"
#include "../syscomm/syscomm.h"
#include "../proclore/proclore.h"
#include "../seek/seek.h"
#include "../shrc/shrc.h"
#include "../iMan/iMan.h"
#include "../activites/activities.h"
#include "../signals/signals.h"
#include "../neonate/neonate.h"
#include "../processes/process.h"
#include "defs.h"

#define true 1
#define in_size 9186
#define MAX_COMMANDS 100
#define MAX_COMMAND_LENGTH 1024
#define RESET   "\x1b[0m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define BOLD    "\033[1m"
#define UNDERLINE "\033[4m"
// #define PATH_MAX 4096

void remove_newline(char *str);

void clear_screen();

// void enable_raw_mode();

void trim(char *str);

void split_command(char *input, char *command_name, char *args);

int run_command(char *line, char *curDir, char *prevDir, char* homeDir, int isBg, int *p_run);

void print_prompt(char *UserName, char *SystemName, char* homeDir);

void parse_and_execute(char *input, char *curDir, char *prevDir, char *homeDir, int *p_run);

#endif // CORE_H
