#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "process.h"
#include "../signals/signals.h"

// Brings a background process to the foreground
int fg(int pid) {

    sigset_t mask, prev_mask;
    
    // Block SIGTTOU to prevent the shell from being stopped when we change the terminal's process group
    sigemptyset(&mask);
    sigaddset(&mask, SIGTTOU);
    sigaddset(&mask, SIGTTIN);
    sigaddset(&mask, SIGTSTP);  // Optionally block SIGTSTP if necessary

    if (sigprocmask(SIG_BLOCK, &mask, &prev_mask) == -1) {
        perror("sigprocmask");
        return -1;
    }

    // Set signal handlers for the foreground process
    signal(SIGINT, handle_sigint);
    signal(SIGTSTP, handle_sigtstp);

    // Check if the process exists
    if (kill(pid, 0) == -1) {
        perror("No such process found");
        return -1;
    }

    // Give the process control of the terminal
    if (tcsetpgrp(STDIN_FILENO, pid) == -1) {
        perror("Error in setting process group");
        return -1;
    }

    // Continue the process if it was stopped
    if (kill(pid, SIGCONT) == -1) {
        perror("Error sending SIGCONT to process");
        return -1;
    }

    set_foreground(pid);

    // Wait for the process to finish or stop
    int status;
    if (waitpid(pid, &status, WUNTRACED) == -1) {
        perror("Error waiting for process");
        return -1;
    }

    // Check how the process terminated and handle accordingly
    if (WIFEXITED(status)) {
        // Process exited normally
        printf("Process %d finished with exit status %d\n", pid, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        // Process was terminated by a signal
        printf("Process %d terminated by signal %d\n", pid, WTERMSIG(status));
    } else if (WIFSTOPPED(status)) {
        // Process was stopped
        printf("Process %d stopped by signal %d\n", pid, WSTOPSIG(status));
    }

    // Restore terminal control to the shell
    if (tcsetpgrp(STDIN_FILENO, getpgrp()) == -1) {
        perror("Error restoring terminal control to shell");
        return -1;
    }

    set_foreground(-1); // Reset the foreground process

    // Unblock the signals
    if (sigprocmask(SIG_SETMASK, &prev_mask, NULL) == -1) {
        perror("sigprocmask");
        return -1;
    }

    return 0;
}

// Continues a stopped process in the background
int bg(int pid)
{
    // Check if the process exists
    if (kill(pid, 0) == -1)
    {
        if (errno == ESRCH)
        {
            fprintf(stderr, "Error: No such process found with PID %d\n", pid);
        }
        else if (errno == EPERM)
        {
            fprintf(stderr, "Error: Permission denied to signal process %d\n", pid);
        }
        else
        {
            perror("kill failed");
        }
        return -1;
    }

    // Send the SIGCONT signal to continue the process
    if (kill(pid, SIGCONT) == -1)
    {
        perror("Error sending SIGCONT to process");
        return -1;
    }

    printf("Process %d continued in background\n", pid);

    return 0;
}
