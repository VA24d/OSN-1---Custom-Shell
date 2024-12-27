#ifndef ACTIVITIES_H
#define ACTIVITIES_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

// Define the maximum number of processes and string sizes
#define MAX_PROCESSES 1024
#define MAX_NAME_LEN 1024
#define MAX_STATE_LEN 1024
#define RESET   "\x1b[0m"
#define RED     "\x1b[31m"



// Structure to hold process information
typedef struct {
    int pid;
    char processName[MAX_NAME_LEN];
    char state[MAX_STATE_LEN];  // "Running" or "Stopped"
} Process;

extern Process process_list[MAX_PROCESSES];
extern int process_count;

// Function declarations
void add_process(int pid, const char* command, const char* state);
void update_process_state(int pid, const char* state);
int check_process_status();
void display_activities();

char * get_process_name(pid_t pid);

#endif // ACTIVITIES_H
