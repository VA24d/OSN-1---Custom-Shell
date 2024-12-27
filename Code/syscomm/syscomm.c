#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include "../activites/activities.h"
#include "../signals/signals.h"
#define RESET "\x1b[0m"
#define YELLOW "\x1b[33m"

#define MAX_BG_PROCESSES 4096
#define COMMAND_BUFFER_SIZE 256

static int background_pids[MAX_BG_PROCESSES]; // Array to store background PIDs
static int background_count = 0;              // Number of background processes

void sigchld_handler()
{
    pid_t pid;
    int status;

    // Use waitpid to non-blockingly check for any finished background processes
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        char *name = get_process_name(pid);
        if (WIFEXITED(status))
        {
            printf("%s (%d) finished normally with exit status %d\n", name, pid, WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("%s (%d) was terminated by signal %d\n", name, pid, WTERMSIG(status));
        }
    }
}

void initialize_time(char *homeDir)
{
    char path[4096];
    snprintf(path, sizeof(path), "%s%s", homeDir, "/.time.txt");

    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0)
    {
        perror("open() error");
        exit(EXIT_FAILURE);
    }
    close(fd);

    FILE *file = fopen(path, "w");
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }
    fclose(file);

    printf("Time initialized\n");
}

pid_t execute_command(char *command, int is_background, char *homeDir, int in_redirect)
{
    // printf("%s\n", command);

    if (is_background)
    {
        struct sigaction sa;
        sa.sa_handler = sigchld_handler;
        sa.sa_flags = SA_RESTART; // To automatically restart interrupted system calls
        sigemptyset(&sa.sa_mask);

        if (sigaction(SIGCHLD, &sa, NULL) == -1)
        {
            perror("sigaction");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        // Foreground process: Remove the SIGCHLD handler
        struct sigaction sa;
        sa.sa_handler = SIG_DFL; // Reset to default handler
        sa.sa_flags = 0;
        sigemptyset(&sa.sa_mask);

        if (sigaction(SIGCHLD, &sa, NULL) == -1)
        {
            perror("sigaction");
            exit(EXIT_FAILURE);
        }
    }

    pid_t pid;
    time_t start, end;

    char command_copy[COMMAND_BUFFER_SIZE];
    snprintf(command_copy, sizeof(command_copy), "%s", command);

    char *args[COMMAND_BUFFER_SIZE];
    char *input_file = NULL; // To hold the input file name if input redirection is detected
    char *token = strtok(command_copy, " ");
    int i = 0;

    // handle ""
    if (strchr(command, '\"') && !strchr(command, '<'))
    {
        int arg_count = 0;
        char *ptr = command;
        char *s_loc;

        while (*ptr != '\0')
        {
            // Skip leading whitespace
            while (*ptr == ' ')
                ptr++;

            if (*ptr == '"')
            {          // Handle quoted arguments
                ptr++; // Skip the opening quote
                s_loc = ptr;
                while (*ptr != '"' && *ptr != '\0')
                    ptr++; // Find closing quote
                if (*ptr == '"')
                    *ptr = '\0'; // Terminate the quoted string
                args[arg_count++] = s_loc;
                ptr++; // Skip the closing quote
            }
            else
            { // Handle regular arguments
                s_loc = ptr;
                while (*ptr != ' ' && *ptr != '\0')
                    ptr++;
                if (*ptr == ' ')
                    *ptr = '\0'; // Terminate the argument string
                args[arg_count++] = s_loc;
                ptr++;
            }
        }
        args[arg_count] = NULL; // Null-terminate the argument array
    }
    else
    {
        while ((token != NULL) && (i < (int)(sizeof(args) / sizeof(args[0]) - 1)))
        {
            if (strcmp(token, "<") == 0) // Detect input redirection symbol "<"
            {
                token = strtok(NULL, " "); // The next token should be the input file name
                if (token != NULL)
                {
                    input_file = token;
                    in_redirect = 1; // Enable input redirection
                    continue;        // Skip adding "<" and the file name to args
                }
            }
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;
    }

    // if (is_background)
    pid = fork();

    if (pid == 0)
    {
        // Child process
        if (is_background)
        {
            setpgid(0, 0); // Set the child's process group to itself

            // Redirect stdin, stdout, and stderr to /dev/null for background processes
            if (!strstr(command, "vim")&&!strstr(command, "nano"))
            {
                int dev_null = open("/dev/null", O_RDWR);
                if (dev_null != -1)
                {
                    dup2(dev_null, STDIN_FILENO);
                    dup2(dev_null, STDOUT_FILENO);
                    dup2(dev_null, STDERR_FILENO);
                    close(dev_null);
                }
                else
                {
                    printf("No dev null\n");
                }
            }
        }

        // Handle input redirection
        if (in_redirect && input_file != NULL)
        {
            int input_fd = open(input_file, O_RDONLY);
            if (input_fd == -1)
            {
                perror("Failed to open input file");
                exit(EXIT_FAILURE);
            }
            dup2(input_fd, STDIN_FILENO); // Redirect stdin to the input file
            close(input_fd);              // Close the file descriptor, as it's no longer needed
        }

        execvp(args[0], args);
        // If execvp fails
        fprintf(stderr, YELLOW "ERROR: '%s' is not a valid command\n" RESET, args[0]);
        // perror("execvp failed");
        exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        if (!is_background)
        {
            add_process(pid, command, "Running");
            set_foreground(pid);
            set_fg_name(command);

            time(&start);

            // if (tcsetpgrp(terminal_fd, pid) == -1)
            // {
            //     perror("Error setting terminal foreground process group");
            // }

            waitpid(pid, NULL, WUNTRACED); // Wait for the process to stop or terminate

            // Restore terminal control to the shell
            // if (tcsetpgrp(terminal_fd, shell_pgid) == -1)
            // {
            //     perror("Error restoring terminal control to shell");
            // }

            time(&end);

            set_foreground(-1);
            set_fg_name("");

            double elapsed = difftime(end, start);
            if (elapsed > 2.0)
            {
                char path[4096];
                snprintf(path, sizeof(path), "%s%s", homeDir, "/.time.txt");

                FILE *file = fopen(path, "w");
                if (!file)
                {
                    perror("fopen() error");
                    exit(EXIT_FAILURE);
                }

                fprintf(file, "%s : %d", args[0], (int)elapsed);
                fclose(file);
            }
            printf("\n");
        }
        else
        {
            if (background_count < MAX_BG_PROCESSES)
            {
                background_pids[background_count++] = pid;
                add_process(pid, command, "Running");
                printf("[%d] %d\n", background_count, pid); // Print PID of the background process
            }
            else
            {
                fprintf(stderr, "Max background processes reached\n");
            }
        }
        return pid;
    }
    else
    {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
}