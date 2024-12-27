#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include "signals.h"
#include <termios.h>
#include "../activites/activities.h"

static struct termios original_termios;

#define RESET "\x1b[0m"
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define CYAN "\x1b[36m"

pid_t foreground_pid = -1;      // Foreground process PID
char foreground_process_n[2048];
pid_t shell_pgid;               // Shell process group ID
int terminal_fd = STDIN_FILENO; // File descriptor for the terminal

void set_foreground(int pid)
{
    foreground_pid = pid;
    return;
}

void set_fg_name(const char *name)
{
    strcpy(foreground_process_n, name);
    return;
}

// void set_raw_mode()
// {
//     struct termios raw;

//     if (tcgetattr(STDIN_FILENO, &original_termios) == -1)
//     {
//         perror("tcgetattr");
//         exit(EXIT_FAILURE);
//     }

//     raw = original_termios;
//     raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
//     raw.c_oflag &= ~(OPOST);
//     raw.c_cflag &= ~(CSIZE | PARENB);
//     raw.c_cflag |= CS8;
//     raw.c_lflag &= ~(ECHO | ICANON | ISIG);

//     if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
//     {
//         perror("tcsetattr");
//         exit(EXIT_FAILURE);
//     }
// }

void reset_mode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios) == -1)
    {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

void init_shell()
{
    // Set the shell's process group
    shell_pgid = getpid(); // Get the PID of the shell, which will also be its process group ID
    if (setpgid(shell_pgid, shell_pgid) == -1)
    {
        perror("Failed to set shell process group");
        exit(1);
    }

    // Give control of the terminal to the shell's process group
    if (tcsetpgrp(terminal_fd, shell_pgid) == -1)
    {
        perror("Failed to set shell as foreground process group");
        exit(1);
    }

    // Ignore background read/write signals
    // signal(SIGTTOU, SIG_IGN);        // Ignore background writes to terminal
    // signal(SIGTTIN, SIG_IGN);        // Ignore background reads from terminal
    // signal(SIGTSTP, handle_sigtstp); // Handle Ctrl+Z for suspending the foreground process

    // Initialize the foreground process PID to -1, indicating no foreground process
    foreground_pid = -1;
}

// Function to send signal to process
void ping_process(pid_t pid, int signal_number)
{
    int signal = signal_number % 32; // Take modulo 32 of signal number

    if (kill(pid, 0) == -1)
    {
        if (errno == ESRCH)
        {
            printf("No such process found\n");
        }
        else
        {
            perror("Error checking process");
        }
        return;
    }

    if (kill(pid, signal) == -1)
    {
        perror("Error sending signal");
    }
    else
    {
        printf(CYAN "Sent signal %d to process with pid %d\n" RESET, signal, pid);
    }
}

// Handle Ctrl+C (SIGINT)
void handle_sigint(int sig)
{
    // printf("\nInterrupt signal received (SIGINT)\n");
    // Implement logic to interrupt the foreground process

    if (foreground_pid > 0)
    {
        // Send SIGINT to the foreground process
        kill(foreground_pid, SIGINT);
        printf(GREEN "Foreground process [%d] killed\n" RESET, foreground_pid);
    }
    else
    {
        printf(RED "\nNo foreground process to interrupt\n" RESET);
    }
}

void handle_sigtstp(int sig)
{
    // Ctrl + Z
    // printf("Ctrl+Z received\n");

    if (foreground_pid > 0)
    {
        // printf("Moving process %d to background\n", foreground_pid);

        // Send SIGTSTP to the foreground process (push it to background and stop it)
        kill(foreground_pid, SIGTSTP);

        setpgid(0, 0);

        // add_process(foreground_pid, foreground_process_n, "Stopped");
        // Move the foreground process to the background
        printf("Process %d stopped and moved to background\n", foreground_pid);


        // Give terminal control back to the shell
        if (tcsetpgrp(terminal_fd, shell_pgid) == -1)
        {
            perror("Error setting terminal foreground process group");
        }

        // Reset foreground process
        foreground_pid = -1;
    }
    else
    {
        printf("\nNo foreground process to stop\n");
    }
}

// Handle Ctrl+D (EOF)
void handle_eof(int *p_run)
{
    printf("Ctrl+D received\n");

    printf("Killing processes and exiting\n");
    check_process_status();
    for (int i = 0; i < process_count; i++)
    {
        kill_process(process_list[i].pid);
    }
    // Implement logic to exit or logout

    p_run = 0;
}

void ping_process_main(char *args)
{

    char pid[100];
    char signal[100];
    char *token;
    token = strtok(args, " ");

    strcpy(pid, token);
    token = strtok(NULL, " "); // Get the next token
    strcpy(signal, token);

    pid_t npid = atoi(pid);
    int nsignal = atoi(signal);

    ping_process(npid, nsignal);
    return;
}

void kill_process(pid_t pid)
{
    // Check if the process exists
    if (kill(pid, 0) == -1)
    {
        if (errno == ESRCH)
        {
            printf("Error: No such process with PID %d\n", pid);
        }
        else
        {
            perror("Error checking process existence");
        }
        return;
    }

    // Send SIGKILL to the process
    if (kill(pid, SIGKILL) == -1)
    {
        perror("Error killing process");
    }
    else
    {
        printf("Process with PID %d has been successfully killed\n", pid);
    }
}