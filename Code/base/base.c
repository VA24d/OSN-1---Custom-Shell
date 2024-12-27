#include "base.h"
#include "stdio.h"

#define BUFFER_SIZE 4096

void clear_screen()
{
    printf("\e[1;1H\e[2J"); 
    return;
}


void remove_newline(char *str)
{
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n')
    {
        str[len - 1] = '\0'; // Replace newline character with null terminator
    }
}


void trim(char *str)
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

void resolve_path(char *path, const char *curDir, const char *prevDir, const char *homeDir)
{
    static char resolved_path[PATH_MAX]; // static so it can be returned safely
    char temp_path[PATH_MAX];

    // If the path is a tilde (~), replace it with home directory
    if (path[0] == '~')
    {
        snprintf(temp_path, sizeof(temp_path), "%s%s", homeDir, path + 1);
    }
    // If the path is a dash (-), replace it with the previous directory
    else if (strcmp(path, "-") == 0)
    {
        snprintf(temp_path, sizeof(temp_path), "%s", prevDir);
    }
    // If the path is relative, use the current directory as the base
    else if (path[0] != '/')
    {
        snprintf(temp_path, sizeof(temp_path), "%s/%s", curDir, path);
    }
    // Otherwise, treat it as an absolute path
    else
    {
        snprintf(temp_path, sizeof(temp_path), "%s", path);
    }

    // Use realpath to resolve any symbolic links, dots (.), or double dots (..)
    if (realpath(temp_path, resolved_path) == NULL)
    {
        perror("Error resolving path");
        return;
    }

    strcpy(path, resolved_path);
    return;
}

void split_command(char *input, char *command_name, char *args)
{
    // Copy input to avoid modifying the original string
    char input_copy[in_size];
    strncpy(input_copy, input, in_size - 1);
    input_copy[in_size - 1] = '\0'; // Ensure null termination

    // Split the input by the first space
    char *token = strtok(input_copy, " ");
    if (token != NULL)
    {
        strncpy(command_name, token, in_size - 1);
        command_name[in_size - 1] = '\0'; // Ensure null termination

        // Get the rest of the input as arguments
        token = strtok(NULL, "");
        if (token != NULL)
        {
            strncpy(args, token, in_size - 1);
            args[in_size - 1] = '\0'; // Ensure null termination
        }
        else
        {
            args[0] = '\0'; // No arguments provided
        }
    }
    else
    {
        command_name[0] = '\0';
        args[0] = '\0';
    }
}

void empty_time(const char *homeDir)
{
    char path[4096];
    snprintf(path, 4096, "%s%s", homeDir, "/.time.txt");
    FILE *file = fopen(path, "w");
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }
    fclose(file);
}

int run_command(char *line, char *curDir, char *prevDir, char *homeDir, int isBg, int *p_run)
{
    // keep track of where file path points to and fix it

    empty_time(homeDir);

    trim(line);

    char command[in_size];
    char arg[in_size];

    char *line_copy = (char *)malloc(strlen(line) + 1);
    strcpy(line_copy, line);

    char file_path[FILENAME_MAX];
    int out_redirect = 0;
    int in_redirect = 0;
    int append_redirect = 0;

    if (strstr(line, ">>"))
    {
        append_redirect = 1;
        char *token = strtok(line_copy, ">>");
        strcpy(line, token);
        trim(line);

        token = strtok(NULL, ">>");
        strcpy(file_path, token);
        trim(file_path);
    }
    else if (strstr(line, ">"))
    {

        out_redirect = 1;
        char *token = strtok(line_copy, ">");
        strcpy(line, token);
        trim(line);

        token = strtok(NULL, ">");
        strcpy(file_path, token);
        trim(file_path);
    }
    else if (strstr(line, "<"))
    {
        in_redirect = 1;

        char *token = strtok(line_copy, "<");
        strcpy(command, token);
        trim(command);

        token = strtok(NULL, "<");
        strcpy(file_path, token);
        trim(file_path);

        // resolve_path(file_path, curDir, prevDir, homeDir);

        FILE *file = fopen(file_path, "r");
        if (file == NULL)
        {
            perror("Error opening file as input");
            return 1;
        }

        // Initialize arg to an empty string
        arg[0] = '\0';

        // Read file contents and accumulate into arg
        char buffer[BUFFER_SIZE];
        while (fgets(buffer, sizeof(buffer), file))
        {
            // Check if the buffer size is smaller than in_size to avoid overflow
            if (strlen(arg) + strlen(buffer) < in_size - 1)
            {
                strcat(arg, buffer);
            }
            else
            {
                fprintf(stderr, "Error: Buffer size too small\n");
                fclose(file);
                return 1;
            }
        }

        fclose(file);

        // If arg is still empty, ensure it's set to an empty string
        if (strlen(arg) == 0)
        {
            arg[0] = '\0';
        }

    }

    if (!in_redirect)
        split_command(line, command, arg);

    // printf("%s", command);

    // printf("%s, %s, %s [out: %d in: %d append: %d]\n", command, arg, file_path, out_redirect, in_redirect, append_redirect);

    int stdout_fd = dup(STDOUT_FILENO);
    if (stdout_fd < 0)
    {
        perror("dup");
        free(line_copy);
        return 1;
    }

    // Handle output redirection
    int fd = -1;
    if (out_redirect || append_redirect)
    {
        int flags = O_WRONLY | O_CREAT;
        if (append_redirect)
            flags |= O_APPEND;
        else
            flags |= O_TRUNC;

        fd = open(file_path, flags, 0644);
        if (fd < 0)
        {
            perror("Error opening file for output redirection");
            close(stdout_fd);
            free(line_copy);
            return 1;
        }

        if (dup2(fd, STDOUT_FILENO) < 0) // Redirect stdout to the file
        {
            perror("dup2");
            close(fd);
            close(stdout_fd);
            free(line_copy);
            return 1;
        }
        close(fd);
    }

    for (int i = 0; i < alias_count; i++)
    {
        // printf("%s %s\n", command, aliases[i].alias_name);
        if (strstr(command, aliases[i].alias_name) != NULL)
        {
            // printf("Alias found\n");
            printf("%s %s\n", aliases[i].command, arg);
            char converted[9000];
            snprintf(converted, sizeof(converted), "%s %s\n", aliases[i].command, arg);
            run_command(converted, curDir, prevDir, homeDir, isBg, p_run);
            return 1;
        }
    }

    for (int i = 0; i < func_count; i++)
    {
        if (strstr(command, functions[i].func_name) != NULL)
        {
            // printf("Function\n");
            char delimiters[] = " ";
            char *token;

            token = strtok(arg, delimiters);
            int count = 1;
            char c_count[20];

            char newline[4096];
            strcpy(newline, functions[i].line);

            while (token != NULL)
            {
                snprintf(c_count, sizeof(c_count), "\"$%d\"", count);
                // printf("%s\n", c_count);

                replace_substring(newline, c_count, token);
                // printf("Token: %s\n", token);     // Print each token
                count++;
                token = strtok(NULL, delimiters); // Pass NULL to continue tokenizing the same string
            }

            printf("%s\n", newline);
            parse_and_execute(newline, curDir, prevDir, homeDir, p_run);
            return 1;
        }
    }

    if (!strcmp(command, "exit") || !strcmp(command, "quit"))
    {
        *p_run=0;
        return 0;
    }
    else if (!strcmp(command, "help"))
    {
        printf("Why do you want help?? Go to https://karthikv1392.github.io/cs3301_osn/mini-projects/mp1\n");
    }
    else if (!strcmp(command, "hop"))
    {
        strcpy(prevDir, curDir);
        hop_main(arg, curDir, prevDir, homeDir);
    }
    else if (!strcmp(command, "reveal"))
    {
        reveal(arg, prevDir);
    }
    else if (!strcmp(command, "log"))
    {
        log_main(arg, curDir, prevDir, homeDir);
    }
    else if (!strcmp(command, "proclore"))
    {
        proclore(arg);
    }
    else if (!strcmp(command, "seek"))
    {
        main_seek(arg, prevDir, homeDir);
    }
    else if (!strcmp(command, "iMan"))
    {
        fetch_man_page(arg);
    }
    else if (!strcmp(command, "shrc"))
    {
        print_loadedSHRC();
    }
    else if (!strcmp(command, "activities"))
    {
        display_activities();
    }
    else if (!strcmp(command, "ping"))
    {
        // display_activities();
        ping_process_main(arg);
    }
    else if (!strcmp(command, "neonate"))
    {
        process_neonate_command(arg);
    }
    else if (!strcmp(command, "fg"))
    {
        pid_t pid = atoi (arg);
        
        fg(pid);
    }
    else if (!strcmp(command, "bg"))
    {
        pid_t pid = atoi (arg);
        
        bg(pid);
    }
    else
    {
        trim(line);
        // pid_t pid = execute_command(line, isBg, homeDir);

        execute_command(line, isBg, homeDir, in_redirect);
        if (isBg == 1)
        {
            printf("\n");
            // handle_background_processes();
        }
    }
    // else
    // {
    //     printf("ERROR : '%s' is not a valid command\n", command);
    // }

    if (dup2(stdout_fd, STDOUT_FILENO) < 0)
    {
        perror("dup2");
        close(stdout_fd);
        free(line_copy);
        return 1;
    }
    close(stdout_fd);

    free(line_copy);

    return 1;
}

void print_prompt(char *UserName, char *SystemName, char *homeDir)
{
    char curDir[4096];

    char path[4096];
    snprintf(path, 4096, "%s%s", homeDir, "/.time.txt");

    FILE *file = fopen(path, "r");
    if (!file)
    {
        perror("fopen() error");
        exit(EXIT_FAILURE);
    }
    char buffer[MAX_COMMAND_LENGTH];
    buffer[0]='\0';
    
    fgets(buffer, sizeof(buffer), file);

    fclose(file);

    // Get the current working directory
    if (getcwd(curDir, sizeof(curDir)) == NULL)
    {
        perror("getcwd() error");
        return;
    }

    // Check if the current directory is the home directory
    if (!strcmp(curDir, homeDir))
    {
        strcpy(curDir, "~");
    }
    else if (strstr(curDir, homeDir) == curDir)
    {
        // Replace <homedir>/<something> with ~/something
        char newDir[4096];
        sprintf(newDir, "~%s", curDir + strlen(homeDir));
        strcpy(curDir, newDir);
    }

    // Print the prompt
    printf(BLUE "<%s" RESET "@" GREEN "%s" RESET ":%s %s> ", UserName, SystemName, curDir, buffer);
}

int countChar(const char *str, char ch)
{
    int count = 0;
    while (*str)
    {
        if (*str == ch)
        {
            count++;
        }
        str++;
    }
    return count;
}

void p_e_normal(char *input, char *curDir, char *prevDir, char *homeDir, int *p_run)
{
    // handle_background_processes();
    char data[4096];
    strcpy(data, input);
    char *delimiter = ";";
    char *line;
    char *line_itr = data;

    while ((line = strtok_r(line_itr, delimiter, &line_itr)))
    {
        char line_internal[4096];
        strncpy(line_internal, line, BUFFER_SIZE - 1);
        line[BUFFER_SIZE - 1] = '\0';
        trim(line_internal);

        int numBg = countChar(line_internal, '&');
        // printf("%s -> Background processes: %d\n", line_internal, numBg);

        char *cmd_token;
        char *cmd_itr = line_internal;

        while ((cmd_token = strtok_r(cmd_itr, "&", &cmd_itr)))
        {
            // printf("->%s\n", cmd_token);

            empty_time(homeDir);

            char command[BUFFER_SIZE];
            strncpy(command, cmd_token, BUFFER_SIZE - 1);
            command[BUFFER_SIZE - 1] = '\0';
            trim(command);

            int isbackground = (numBg > 0) ? 1 : 0;
            if (isbackground)
                numBg--;

            if (strlen(command) > 0)
            {
                *p_run = run_command(command, curDir, prevDir, homeDir, isbackground, p_run);
            }
            // printf("%s", cmd_itr);
        }
    }
}

void p_e_pipe(char *input, char *curDir, char *prevDir, char *homeDir, int *p_run)
{
    // handle_background_processes();

    char data[4096];
    strcpy(data, input);

    // Count the number of pipes in the input
    int num_pipes = countChar(data, '|');
    
    if (num_pipes == 0) {
        // No pipes, run as a single command
        run_command(data, curDir, prevDir, homeDir, 0, p_run);
        return;
    }

    // Handle pipe case
    char *commands[num_pipes + 1];  // To store commands split by '|'
    char *cmd = strtok(data, "|");
    int i = 0;

    while (cmd != NULL) {
        commands[i++] = cmd;
        cmd = strtok(NULL, "|");
    }

    // Now execute the commands with piping
    int pipefd[2];  // Pipe file descriptors
    int fd_in = 0;  // Input file descriptor for the next command

    for (i = 0; i <= num_pipes; i++) {
        pipe(pipefd);  // Create a pipe for each command

        pid_t pid = fork();
        if (pid == 0) {
            // Child process

            // If not the first command, redirect input
            if (fd_in != 0) {
                dup2(fd_in, 0);
                close(fd_in);
            }

            // If not the last command, redirect output to the pipe
            if (i != num_pipes) {
                dup2(pipefd[1], 1);
            }

            close(pipefd[0]);  // Close read end of the pipe

            // Execute the current command
            if (strlen(commands[i])==0)
            {
                printf("Invalid use of pipe\n");
                exit(0); 
            }
            *p_run = run_command(commands[i], curDir, prevDir, homeDir, 0, p_run);
            exit(0);
        } else {
            // Parent process

            // Close the write end of the pipe in the parent process
            close(pipefd[1]);

            // Save the read end of the pipe as input for the next command
            fd_in = pipefd[0];
        }
    }

    // Wait for all child processes to finish
    for (i = 0; i <= num_pipes; i++) {
        wait(NULL);
    }
}

void parse_and_execute(char *input, char *curDir, char *prevDir, char *homeDir, int *p_run) {
    if (strchr(input, '|'))
    {
        p_e_pipe(input, curDir, prevDir, homeDir, p_run);
    }
    else
    {
        p_e_normal(input, curDir, prevDir, homeDir, p_run);
    }
    
}