#include "proclore.h"
#include <ctype.h>

#define RESET "\x1b[0m"
#define RED "\x1b[31m"
#define CYAN "\x1b[36m"

#define MAX_PATH 4096

void trim2(char *str)
{
    char *start = str;
    char *end;

    // Move start pointer to the first non-space character
    while (isspace((unsigned char)*start))
    {
        start++;
    }

    // All spaces case
    if (*start == '\0')
    {
        str[0] = '\0';
        return;
    }

    // Move end pointer to the last non-space character
    end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end))
    {
        end--;
    }

    // Write the null terminator
    *(end + 1) = '\0';

    // Move the trimmed string back to the original array
    memmove(str, start, end - start + 2);
}

int is_foreground(pid_t pid)
{
    pid_t process_group = getpgid(pid);               // Get the process group of the given pid
    pid_t fg_process_group = tcgetpgrp(STDIN_FILENO); // Get the foreground process group of the terminal

    if (process_group == fg_process_group)
    {
        return 1; // The process is in the foreground
    }
    else
    {
        return 0; // The process is in the background
    }
}

void print_process_info(pid_t pid)
{
    char path[MAX_PATH];
    // char status[256];
    char exe_path[MAX_PATH];
    FILE *file;
    char line[256];

    // Variables to store process information
    char pid_str[256] = "";
    char proc_status[256] = "";
    char proc_group[256] = "";
    char virtual_memory[256] = "";

    // Construct the path to the status file
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    file = fopen(path, "r");
    if (!file)
    {
        perror(RED "Could not open proc. proclore only works on Linux (use parallels (⁠╥⁠﹏⁠╥⁠) )" RESET);
        return;
    }

    // Read the process status
    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "State:", 6) == 0)
        {
            // printf("%d", is_foreground(pid));
            char dest[6]; // Allocate enough space for the copied characters plus the null terminator

            // Copy only the first 5 characters from src to dest
            strncpy(dest, line+7, 2);

            // Manually add null terminator to the destination string
            dest[3] = '\0';

            dest[2] = is_foreground(pid) ? '+' : ' ';

            snprintf(proc_status, sizeof(proc_status), "\033[94mProcess status:\033[0m %s \n\n",  dest);
        }
        else if (strncmp(line, "Pid:", 4) == 0)
        {
            snprintf(pid_str, sizeof(pid_str), "\033[94mpid:\033[0m %s\n", line + 5);
        }
        else if (strncmp(line, "PPid:", 5) == 0)
        {
            snprintf(proc_group, sizeof(proc_group), "\033[94mProcess Group:\033[0m %s\n", line + 6);
        }
        else if (strncmp(line, "VmSize:", 7) == 0)
        {
            snprintf(virtual_memory, sizeof(virtual_memory), "\033[94mVirtual memory:\033[0m %s\n", line + 8);
        }
    }
    fclose(file);

    // Construct the path to the executable file
    snprintf(path, sizeof(path), "/proc/%d/exe", pid);
    ssize_t len = readlink(path, exe_path, sizeof(exe_path) - 1);
    if (len != -1)
    {
        exe_path[len] = '\0';
    }
    else
    {
        snprintf(exe_path, sizeof(exe_path), "(unknown)");
    }

    // Print process information in the desired order
    printf("%s", pid_str);
    printf("%s", proc_status);
    printf("%s", proc_group);
    printf("%s", virtual_memory);
    printf("\033[94mExecutable path:\033[0m %s\n\n", exe_path);
}

void proclore(char *pid)
{
    trim2(pid);
    int n_pid;

    if (strlen(pid) == 0)
        n_pid = getpid();
    else
        n_pid = atoi(pid);

    print_process_info(n_pid);

    return;
}
