#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "neonate.h"


int running = 1;  // Global flag to control the loop

// Function to make terminal input non-blocking
void set_nonblocking_input()
{
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);  // Get terminal attributes
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);  // Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);  // Make input non-blocking
}

// Function to restore terminal input to blocking mode
void restore_terminal_input()
{
    struct termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);  // Get terminal attributes
    oldt.c_lflag |= (ICANON | ECHO);  // Enable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, 0);  // Make input blocking
}

// Function to get the most recently created process PID
int get_most_recent_pid()
{
    DIR *proc_dir;
    struct dirent *entry;
    int max_pid = -1;

    proc_dir = opendir("/proc");
    if (!proc_dir)
    {
        perror("opendir /proc");
        return -1;
    }

    // Iterate through /proc to find the highest numerical directory
    while ((entry = readdir(proc_dir)) != NULL)
    {
        if (entry->d_type == DT_DIR)  // Only consider directories
        {
            int pid = atoi(entry->d_name);
            if (pid > 0 && pid > max_pid)
            {
                max_pid = pid;
            }
        }
    }

    closedir(proc_dir);
    return max_pid;
}

// Function to handle the neonate process printing PIDs
void neonate(unsigned int time_arg)
{
    set_nonblocking_input();  // Enable non-blocking input for 'x' key detection

    while (running)
    {
        int recent_pid = get_most_recent_pid();
        if (recent_pid != -1)
        {
            printf("%d\n", recent_pid);
        }
        else
        {
            printf("Failed to retrieve recent PID\n");
        }

        // Sleep for the specified interval
        sleep(time_arg);

        // Check if the user pressed 'x'
        char ch = getchar();
        if (ch == 'x' || ch == 'X')
        {
            printf("Terminating on 'x' key press...\n");
            running = 0;
        }
    }

    running=1; //fixed
    restore_terminal_input();  // Restore terminal settings
}


void process_neonate_command(char *input) {
    // printf("Neonate ");
    char *token;
    char *args[3];  // Array to hold tokenized arguments
    int count = 0;

    // Tokenize input string using space as the delimiter
    token = strtok(input, " ");
    while (token != NULL && count < 3) {
        args[count] = token;  // Store the tokens in args array
        count++;
        token = strtok(NULL, " ");
    }

    // Now args[0] should be "neonate", args[1] should be "-n", and args[2] should be the time_arg

    // Check if the command is valid
    if (count != 2 || strcmp(args[0], "-n") != 0) {
        fprintf(stderr, "Usage: neonate -n <time_arg>\n");
        // printf("Usage: neonate -n <time_arg>\n");
        return;
    }

    // Convert time_arg from string to integer
    unsigned time_arg = (long)atoi(args[1]);
    if (time_arg <= 0) {
        fprintf(stderr, "Invalid time_arg: must be a positive integer\n");
        return;
    }

    // Call the neonate function with the parsed time_arg
    neonate(time_arg);
}