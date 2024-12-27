#include "activities.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

Process process_list[MAX_PROCESSES]; // Array to store processes
int process_count = 0;               // Count of current processes

int is_linux_system()
{
    struct stat info;

    // Check if /proc exists and is a directory
    if (stat("/proc", &info) == 0 && S_ISDIR(info.st_mode))
    {
        return 1; // It's a Linux/Unix-like system (since /proc exists and is a directory)
    }
    else
    {
        return 0; // Not a Linux system
    }
}

// Function to add a new process to the list
void add_process(int pid, const char *command, const char *state)
{
    if (process_count < MAX_PROCESSES)
    {
        process_list[process_count].pid = pid;
        strncpy(process_list[process_count].processName, command, MAX_NAME_LEN);
        strncpy(process_list[process_count].state, state, MAX_STATE_LEN);
        process_count++;
    }
    else
    {
        printf(RED "Error: Maximum number of processes reached.\n" RESET);
    }
}

// Function to update the state of a process
void update_process_state(int pid, const char *state)
{
    for (int i = 0; i < process_count; i++)
    {
        if (process_list[i].pid == pid)
        {
            strncpy(process_list[i].state, state, MAX_STATE_LEN);
            break;
        }
    }
}

// Function to sort the process list lexicographically by command name
int compare_processes(const void *a, const void *b)
{
    Process *pa = (Process *)a;
    Process *pb = (Process *)b;
    return strcmp(pa->processName, pb->processName);
}

void remove_process_from_list(int index)
{
    // Shift all elements after the index to the left
    for (int i = index; i < process_count - 1; i++)
    {
        process_list[i] = process_list[i + 1];
    }
    // Decrease the process count
    process_count--;
}

int check_process_status()
{
    char path[1024];
    char line[256];
    char proc_status[256];

    for (int i = 0; i < process_count; i++)
    {
        // Construct the path to the /proc/[pid]/stat file
        snprintf(path, sizeof(path), "/proc/%d/stat", process_list[i].pid);
        // printf("%s", path);

        if (!is_linux_system())
        {
            printf(RED "This command can only be used on Linux (or a Unix-based system with /proc).\n" RESET);
            return 0;
        }

        // Open the stat file
        FILE *file = fopen(path, "r");

        if (!file)
        {
            // printf("Removing %d\n", process_list[i].pid);
            // If opening fails, assume the process has stopped
            update_process_state(process_list[i].pid, "Stopped");
            remove_process_from_list(i);
            i--; // Adjust the index because the array has been shifted
            continue;
        }

        while (fgets(line, sizeof(line), file))
        {
            if (strncmp(line, "State:", 6) == 0)
            {
                snprintf(proc_status, sizeof(proc_status), "%s", line + 7);
                printf("%s", line);
            }
        }
        fclose(file);

        // printf("%s", proc_status);

        // Update the process state based on the read value
        if (strchr(proc_status, 'R'))
        {
            // 'R' for Running
            update_process_state(process_list[i].pid, "Running");
        }
        // else if (strchr(proc_status, 'T'))
        // {
        //     remove_process_from_list(i);
        //     i--; // Adjust the index because the array has been shifted
        // }
        else
        {
            // For any other states (zombie, dead, etc.), assume it's stopped
            update_process_state(process_list[i].pid, "Stopped");
            // remove_process_from_list(i);
            // i--; // Adjust the index because the array has been shifted
        }
    }

    return 1;
}

// Function to display the list of processes
void display_activities()
{
    // First, check and update the status of all processes
    if (check_process_status())
    {
        // Sort the process list lexicographically by command name
        qsort(process_list, process_count, sizeof(Process), compare_processes);

        // Print the list of processes
        if (process_count == 0)
        {
            printf("No processes are started\n");
            return;
        }
        for (int i = 0; i < process_count; i++)
        {
            printf("[%d] : %s - %s\n", process_list[i].pid, process_list[i].processName, process_list[i].state);
        }
    }
}

char *get_process_name(pid_t pid)
{
    for (int i = 0; i < process_count; i++)
    {
        if (pid == process_list[i].pid)
        {
            return process_list[i].processName;
        }
    }

    return NULL;
}
