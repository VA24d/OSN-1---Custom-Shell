#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#define BLUE "\033[34m"
#define GREEN "\033[32m"
#define RESET "\033[0m"
#define LIM_PATH 9000

int is_directory(const char *path)
{
    struct stat path_stat;
    if (stat(path, &path_stat) != 0)
    {
        // Print an error message if stat fails
        fprintf(stderr, "Error accessing %s: %s\n", path, strerror(errno));
        return 0; // Consider path not a directory if stat fails
    }
    return S_ISDIR(path_stat.st_mode);
}

int has_permissions(const char *path, int mode)
{
    return access(path, mode) == 0;
}

void parse_input(char *input, char *flags, char *search, char *target_directory, char *prevDir, char *homeDir)
{
    flags[0] = '\0';
    search[0] = '\0';
    target_directory[0] = '\0';

    char *token = strtok(input, " ");
    int flag_done = 0;

    while (token)
    {
        if (!flag_done && token[0] == '-')
        {
            strcat(flags, token);
            token = strtok(NULL, " ");
            continue;
        }

        if (flag_done == 0)
        {
            strcpy(search, token);
            flag_done = 1;
        }
        else
        {
            if (strcmp(token, "~") == 0)
            {
                strcpy(target_directory, homeDir);
            }
            else if (strcmp(token, "-") == 0)
            {
                strcpy(target_directory, prevDir);
            }
            else
            {
                strcpy(target_directory, token);
            }
        }

        token = strtok(NULL, " ");
    }

    if (strlen(target_directory) == 0)
    {
        strcpy(target_directory, ".");
    }
}

void print_file_contents(const char *file_path)
{
    // printf("");
    FILE *file = fopen(file_path, "r"); // Open the file in read mode

    if (!file)
    { // Check if the file was successfully opened
        fprintf(stderr, "Error opening file %s: %s\n", file_path, strerror(errno));
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file))
    {                       // Read each line from the file
        printf("%s", line); // Print the line
    }

    fclose(file); // Close the file after reading
}

void print_all_files(const char *directory)
{
    DIR *dir = opendir(directory);
    if (!dir)
    {
        fprintf(stderr, "Error opening directory %s: %s\n", directory, strerror(errno));
        return;
    }

    struct dirent *entry;
    char full_path[4096];

    while ((entry = readdir(dir)) != NULL)
    {
        // Skip the special entries "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        snprintf(full_path, sizeof(full_path), "%s/%s", directory, entry->d_name);

        // Print the full path of the file/directory
        if (is_directory(full_path))
            printf(BLUE "%s\n" RESET, full_path);
        else
            printf(GREEN "%s\n" RESET, full_path);
        if (is_directory(full_path))
        {
            // Recursively process subdirectories
            print_all_files(full_path);
        }
    }

    closedir(dir);
}

int rec_seek(const char *flags, const char *search, const char *target_directory)
{
    int only_dirs = 0, only_files = 0, execute_flag = 0;
    struct dirent *entry;
    DIR *dir;
    char full_path[LIM_PATH];
    char resolved_target_directory[4096];
    int match_count = 0;

    if (strchr(flags, 'd'))
        only_dirs = 1;
    if (strchr(flags, 'f'))
        only_files = 1;
    if (strchr(flags, 'e'))
        execute_flag = 1;
    if (only_dirs && only_files)
    {
        printf("\033[31mInvalid flags!\033[0m\n");
        return 0;
    }



    realpath(target_directory, resolved_target_directory);

    // printf("Flags: %s\n", flags);
    // printf("Search phrase: %s\n", search);
    // printf("Directory: %s\n", resolved_target_directory);

    dir = opendir(resolved_target_directory);
    if (!dir)
    {
        printf("\033[31mNo match found!\033[0m\n");
        return 0;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        snprintf(full_path, sizeof(full_path), "%s/%s", resolved_target_directory, entry->d_name);
        // printf("%s\n", full_path);
        // printf("%s\n", entry->d_name);
        if (is_directory(full_path))
        {
            // printf(" (Directory)\n");
            match_count += rec_seek(flags, search, full_path);
        }

        if (strcmp(entry->d_name, search) == 0)
        {
            if (is_directory(full_path))
            {
                if (!only_files)
                {
                    // printf("%s%s/%s\n", BLUE, entry->d_name, RESET);
                    if (!only_dirs||is_directory(full_path))
                        printf("%s%s%s\n", BLUE, full_path, RESET);

                    if (only_dirs == 0)
                    {
                        print_all_files(full_path);
                    }
                    match_count++;
                }

                if (execute_flag && !only_files) // change here
                {
                    print_file_contents(full_path);
                    closedir(dir);
                    return match_count;
                }
            }
            else
            {
                if (!only_dirs)
                {
                    // printf("%s%s%s\n", GREEN, entry->d_name, RESET);
                    printf("%s%s%s\n", GREEN, full_path, RESET);
                    match_count++;
                    if (execute_flag)
                    {
                        printf(">");
                        print_file_contents(full_path);
                        closedir(dir);
                        return match_count;
                    };
                }
            }
        }
    }

    closedir(dir);

    return match_count;
}

void seek(const char *flags, const char *search, const char *target_directory)
{
    int only_dirs = 0, only_files = 0, execute_flag = 0;
    struct dirent *entry;
    DIR *dir;
    char full_path[LIM_PATH];
    char resolved_target_directory[4096];
    int match_count = 0;

    if (flags)
    {
        if (strchr(flags, 'd'))
            only_dirs = 1;
        if (strchr(flags, 'f'))
            only_files = 1;
        if (strchr(flags, 'e'))
            execute_flag = 1;

        if (only_dirs && only_files)
        {
            printf("\033[31mInvalid flags!\033[0m\n");
            return;
        }
    }

    realpath(target_directory, resolved_target_directory);

    // printf("Flags: %s\n", flags);
    // printf("Search phrase: %s\n", search);
    // printf("Directory: %s\n", resolved_target_directory);

    dir = opendir(resolved_target_directory);
    if (!dir)
    {
        printf("\033[31mNo match found!\033[0m\n");
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        snprintf(full_path, sizeof(full_path), "%s/%s", resolved_target_directory, entry->d_name);
        // printf("%s\n", full_path);
        // printf("%s\n", entry->d_name);
        if (is_directory(full_path))
        {
            // printf(" (Directory)\n");
            match_count += rec_seek(flags, search, full_path);
        }

        if (strcmp(entry->d_name, search) == 0)
        {
            if (is_directory(full_path))
            {
                // printf(" (Directory)\n");
                if (!only_files)
                {
                    // printf("%s%s/%s\n", BLUE, entry->d_name, RESET);
                    printf("%s%s%s\n", BLUE, full_path, RESET);
                    if (!only_dirs)
                        print_all_files(full_path);
                    match_count++;
                }
                if (execute_flag && !only_files)
                {
                    print_file_contents(full_path);
                    closedir(dir);
                    return;
                }
            }
            else
            {
                if (!only_dirs)
                {
                    // printf("%s%s%s\n", GREEN, entry->d_name, RESET);
                    printf("%s%s%s\n", GREEN, full_path, RESET);
                    match_count++;
                    if (execute_flag)
                    {
                        print_file_contents(full_path);
                        closedir(dir);
                        return;
                    }
                }
            }
        }
    }

    closedir(dir);

    if (match_count == 0)
    {
        printf("\033[31mNo match found!\033[0m\n");
    }
}

void main_seek(char *input, char *prevDir, char *homeDir)
{
    char flags[10];
    char search[256];
    char target_directory[4096];

    parse_input(input, flags, search, target_directory, prevDir, homeDir);

    if (strlen(search) == 0)
    {
        printf("\033[31mNo search string provided\033[0m\n");
        return;
    }

    seek(flags, search, target_directory);
    printf("\n");
}
