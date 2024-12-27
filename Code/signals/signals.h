#ifndef COMMANDS_H
#define COMMANDS_H
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

extern pid_t foreground_pid;  // Foreground process PID
extern pid_t shell_pgid;      // Shell process group ID
extern int terminal_fd;       // Terminal file descriptor

void init_shell();
void set_foreground(int pid);
void set_fg_name(const char *name);
void ping_process_main(char *args);
void ping_process(pid_t pid, int signal_number);
void handle_sigint(int sig);

void kill_process(pid_t pid);

void handle_sigtstp(int sig);
void handle_eof(int * p_run);

#endif // COMMANDS_H
