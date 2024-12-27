#include "reveal.h"
#define RESET "\x1b[0m"
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define BLUE "\x1b[34m"

// Function to check if a file is hidden
int is_hidden(const char *name)
{
    return name[0] == '.';
}

// Function to print file or directory details with color coding
void print_file_info(const char *path, const char *name, int show_extra)
{
    struct stat st;
    char fullpath[PATH_MAX];

    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, name);

    if (stat(fullpath, &st) != 0)
    {
        perror("stat() error");
        return;
    }

    if (S_ISDIR(st.st_mode))
    {
        printf(BLUE "%s" RESET, name); // Blue for directories
    }
    else if (S_ISREG(st.st_mode))
    {
        if (st.st_mode & S_IXUSR)
        {
            printf(GREEN "%s" RESET, name); // Green for executables
        }
        else
        {
            printf("%s", name); // White for regular files
        }
    }
    else if (S_ISLNK(st.st_mode))
    {
        printf(RED "%s" RESET, name); // Cyan for symlinks (optional)
    }
    else
    {
        printf("%s", name); // Default color for other types
    }

    if (is_hidden(name))
    {
        printf("\033[35m(Hidden)\033[0m"); // Cyan for hidden files
    }

    if (show_extra)
    {
        printf("\t");

        // Print detailed information
        char timebuf[64];
        struct passwd *pw = getpwuid(st.st_uid);
        struct group *gr = getgrgid(st.st_gid);
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&st.st_mtime));

        // printf("Permissions: ");
        printf((S_ISDIR(st.st_mode)) ? "d" : "-");
        printf((st.st_mode & S_IRUSR) ? "r" : "-");
        printf((st.st_mode & S_IWUSR) ? "w" : "-");
        printf((st.st_mode & S_IXUSR) ? "x" : "-");
        printf((st.st_mode & S_IRGRP) ? "r" : "-");
        printf((st.st_mode & S_IWGRP) ? "w" : "-");
        printf((st.st_mode & S_IXGRP) ? "x" : "-");
        printf((st.st_mode & S_IROTH) ? "r" : "-");
        printf((st.st_mode & S_IWOTH) ? "w" : "-");
        printf((st.st_mode & S_IXOTH) ? "x" : "-");

        printf(" %u %s %s %lld %s\n",
               st.st_nlink,
               pw ? pw->pw_name : "unknown",
               gr ? gr->gr_name : "unknown",
               (long long) st.st_size,
               timebuf);
    }
    else
    {
        printf("\n");
    }
}

// Function to list files in a directory with flags
void list_directory(const char *path, int show_all, int show_extra)
{
    struct dirent **namelist;
    int n;

    // Get the directory contents
    n = scandir(path, &namelist, NULL, alphasort);
    if (n < 0)
    {
        perror("scandir() error");
        return;
    }

    // Iterate through the directory entries
    for (int i = 0; i < n; i++)
    {
        if (!show_all && is_hidden(namelist[i]->d_name))
        {
            free(namelist[i]);
            continue;
        }

        print_file_info(path, namelist[i]->d_name, show_extra);

        free(namelist[i]);
    }
    free(namelist);
}

void reveal(char *arg, char *prevDir)
{
    // char *flags = NULL;
    // char *path = NULL;
    char curDir[PATH_MAX];
    char homeDir[PATH_MAX];
    char expandedPath[PATH_MAX + 10];

    // Get the current directory
    if (getcwd(curDir, sizeof(curDir)) == NULL)
    {
        perror("getcwd() error");
        return;
    }

    // Get the home directory from the environment
    strcpy(homeDir, getenv("HOME"));

    // Initialize flag values
    int show_all = 0;
    int show_extra = 0;

    // Extract flags and path
    char *temp_arg = strdup(arg); // Duplicate arg to avoid modifying the original
    char *token = strtok(temp_arg, " ");
    // char *flag_str = NULL;
    char *path_str = NULL;

    while (token)
    {
        if (token[0] == '-')
        {
            // flag_str = token;
            char temp_str[100];
            strcpy(temp_str, token);
            show_all = show_all==0?(strstr(temp_str, "a") != NULL):show_all;
            show_extra = show_extra==0?(strstr(temp_str, "l") != NULL):show_extra;
        }
        else
        {
            path_str = token;
            break; // No need to process further tokens
        }
        token = strtok(NULL, " ");
    }

    // Parse flags
    // if (flag_str)
    // {
    //     show_all = (strstr(flag_str, "a") != NULL);
    //     show_extra = (strstr(flag_str, "l") != NULL);
    // }

    // Determine path
    if (path_str == NULL)
    {
        path_str = ".";
    }

    // Handle special symbols
    if (strcmp(path_str, ".") == 0)
    {
        path_str = curDir;
    }
    else if (strcmp(path_str, "..") == 0)
    {
        long unsigned int length;
        length = snprintf(expandedPath, sizeof(expandedPath), "%s/..", curDir);

        if (length >= sizeof(expandedPath))
        {
            fprintf(stderr, "Warning: Path truncated. Ensure curDir is not too long.\n");
            // Handle truncation case if necessary
        }
        path_str = expandedPath;
    }
    else if (strcmp(path_str, "~") == 0)
    {
        path_str = homeDir;
    }
    else if (strcmp(path_str, "-") == 0)
    {
        // path_str=malloc(4096*sizeof(char))
        strcpy(path_str, prevDir); // Previous directory
        printf("%s-%s\n", path_str, prevDir);
        // issue
    }

    // List the directory contents
    list_directory(path_str, show_all, show_extra);

    free(temp_arg); // Free the duplicated argument string
}
