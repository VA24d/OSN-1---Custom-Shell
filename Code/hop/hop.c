#include "hop.h"
#include "string.h"
#include "stdlib.h"

void hop(char *arg, char *curDir, char *prevDir, char *homeDir)
{
    // Print the original argument for debugging
    // printf("\'%s\'\n", arg);

    if (!strcmp(arg, ".") || !strcmp(arg, "./"))
    {
        printf("Moved to \033[34m%s\033[0m\n", curDir);
        return;
    }
    else if (!strcmp(arg, "~"))
    {
        strcpy(arg, homeDir);
    }
    else if (!strcmp(arg, "-"))
    {
        strcpy(arg, prevDir);
    }
    else if (arg[0] == '~' && arg[1] == '/')
    {
        // Convert ~/something to home_directory/something
        char new[4096];
        strcpy(new, homeDir);
        strcat(new, arg + 1); // Append the rest of the path after the ~
        strcpy(arg, new);     // Update arg to point to the new path
    }

    // Attempt to change the directory
    if (chdir(arg) != 0)
    {
        perror("chdir() error");
        return;
    }

    // Get the current directory after hopping
    char Dir[PATH_MAX];
    if (getcwd(Dir, sizeof(Dir)) == NULL)
    {
        perror("getcwd() error");
        return;
    }

    // Print the new directory path
    printf("Moved to \033[34m%s\033[0m\n", Dir);
    strcpy(prevDir, arg);
}

void hop_main(char *arg, char *curDir, char *prevDir, char *homeDir)
{
    // printf("Hopping to \'%s\'[%d]\n", arg, strlen(arg));
    char str[in_size];
    strcpy(str, arg);

    // Space is used as the delimiter to split
    char delimiter[] = " ";

    // Declare empty string to store token
    char *token;
    // Get the first token
    if (str[strlen(str) - 1] == '\n')
    {
        str[strlen(str) - 1] = '\0';
    }

    if (strlen(str) == 0)
    {
        strcpy(str, "~");
    }

    token = strtok(str, delimiter);
    while (token)
    {
        char jump[1024];
        strcpy(jump, token);
        hop(jump, curDir, prevDir, homeDir);
        token = strtok(NULL, delimiter);
    }
    return;
}
