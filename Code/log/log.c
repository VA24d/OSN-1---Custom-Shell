#include "log.h"
#define RESET "\x1b[0m"
#define RED "\x1b[31m"

// Helper function to construct the full log file path
void construct_log_path(char *path, const char *homeDir)
{
    snprintf(path, 4096, "%s%s", homeDir, LOG_FILE);
}

// Function to initialize the log file
void initialize_log(char *homeDir)
{
    char path[4096];
    construct_log_path(path, homeDir);

    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0)
    {
        perror(RED "Could not create LOG file" RESET);
        exit(EXIT_FAILURE);
    }
    close(fd);

    printf("Log initialized\n");
}

// Function to get the total number of commands in the log file
int get_log_size(char *homeDir)
{
    char path[4096];
    construct_log_path(path, homeDir);

    FILE *file = fopen(path, "r");
    if (!file)
    {
        perror("fopen() error");
        return -1;
    }

    int count = 0;
    char buffer[MAX_COMMAND_LENGTH];
    while (fgets(buffer, sizeof(buffer), file))
    {
        count++;
    }
    fclose(file);

    return count;
}

// Function to append a command to the log file
void log_command(const char *command, char *homeDir)
{
    char path[4096];
    construct_log_path(path, homeDir);

    if (strlen(command) == 0 || strstr(command, "log") != NULL)
    {
        return;
    }

    int log_size = get_log_size(homeDir);
    FILE *file = fopen(path, "a");
    if (!file)
    {
        perror("fopen() error");
        exit(EXIT_FAILURE);
    }

    // If log size exceeds max history, truncate the log file
    if (log_size >= MAX_HISTORY)
    {
        FILE *temp_file = fopen("temp.log", "w");
        if (!temp_file)
        {
            perror("fopen() error");
            exit(EXIT_FAILURE);
        }

        char buffer[MAX_COMMAND_LENGTH];
        int skipped_lines = log_size - MAX_HISTORY + 1;

        FILE *original_file = fopen(path, "r");
        if (!original_file)
        {
            perror("fopen() error");
            exit(EXIT_FAILURE);
        }

        // Skip the oldest entries
        for (int i = 0; i < skipped_lines; i++)
        {
            if (!fgets(buffer, sizeof(buffer), original_file))
                break;
        }

        // Copy remaining entries to temp file
        while (fgets(buffer, sizeof(buffer), original_file))
        {
            fputs(buffer, temp_file);
        }

        fclose(original_file);
        fclose(temp_file);

        // Replace old log file with the new truncated file
        if (remove(path) != 0 || rename("temp.log", path) != 0)
        {
            perror("remove/rename error");
            exit(EXIT_FAILURE);
        }

        file = fopen(path, "a");
        if (!file)
        {
            perror("fopen() error");
            exit(EXIT_FAILURE);
        }
    }

    fprintf(file, "%s\n", command);
    fclose(file);
}

// Function to print the command log
void print_log(char *homeDir)
{
    char path[4096];
    construct_log_path(path, homeDir);

    FILE *file = fopen(path, "r");
    if (!file)
    {
        perror("fopen() error");
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_COMMAND_LENGTH];
    while (fgets(buffer, sizeof(buffer), file))
    {
        printf("%s", buffer);
    }

    if (strlen(buffer) == 0)
        printf("Log is empty. Try out some commands\n");

    fclose(file);
    return;
}

// Function to clear the log file
void purge_log(char *homeDir)
{
    char path[4096];
    construct_log_path(path, homeDir);

    FILE *file = fopen(path, "w");
    if (!file)
    {
        perror("fopen() error");
        exit(EXIT_FAILURE);
    }
    fclose(file);
    printf("Log cleared\n");
    return;
}

// Function to execute a command by index
void execute_log_command(int index, char *curDir, char *prevDir, char *homeDir)
{
    char path[4096];
    construct_log_path(path, homeDir);

    FILE *file = fopen(path, "r");
    if (!file)
    {
        perror("fopen() error");
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_COMMAND_LENGTH];
    int current_index = get_log_size(homeDir);

    if (current_index == 0)
    {
        printf(RED "The LOG is empty. Run some commands" RESET);
    }

    while (fgets(buffer, sizeof(buffer), file))
    {
        if (current_index == index)
        {
            printf("Executing "BLUE"%s"RESET, buffer);
            // run_command(buffer, curDir, prevDir, homeDir, 0);
            int rand = 0;
            int *p_ran = &rand;
            parse_and_execute(buffer, curDir, prevDir, homeDir, p_ran);
            fclose(file);
            return;
        }
        current_index--;
    }

    printf("Index out of range\n");
    fclose(file);
    return;
}

void log_main(const char *arg, char *curDir, char *prevDir, char *homeDir)
{
    if (arg == NULL || strlen(arg) == 0)
    {
        // Print the log
        print_log(homeDir);
    }
    else if (strncmp(arg, "purge", 5) == 0 && (arg[5] == '\0' || isspace(arg[5])))
    {
        // Purge the log
        purge_log(homeDir);
    }
    else if (strncmp(arg, "execute ", 8) == 0)
    {
        // Extract index and execute
        int index = atoi(arg + 8);
        if (index >= 0)
        {
            execute_log_command(index, curDir, prevDir, homeDir);
        }
        else
        {
            fprintf(stderr, "Invalid index\n");
        }
    }
    else
    {
        fprintf(stderr, "Invalid command\n");
    }
    return;
}
