#ifndef SHELL_H
#define SHELL_H

void initialize_time(char* homeDir);

// Function to execute a command in foreground or background
pid_t execute_command(const char *command, int is_background, char* homeDir, int in_redirect);

// Function to handle background process completion
// void handle_background_processes();

#endif // SHELL_H
