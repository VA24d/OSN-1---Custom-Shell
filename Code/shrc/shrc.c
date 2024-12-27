#include "shrc.h"

int alias_count = 0;
int func_count = 0;

Alias aliases[MAX_ALIASES];
Function functions[MAX_FUNCTIONS];

void trim_newline(char *line)
{
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n')
    {
        line[len - 1] = '\0';
    }
}

void trim_spaces(char *str)
{
    char *p = str;
    char *q = str;
    char *start, *end;

    // Trim leading spaces
    while (isspace((unsigned char)*p))
        p++;
    if (*p == '\0')
    {
        *str = '\0'; // String was all spaces
        return;
    }

    // Trim trailing spaces
    end = p + strlen(p) - 1;
    while (end > p && isspace((unsigned char)*end))
        end--;
    end[1] = '\0';

    // Remove extra spaces between words
    start = p;
    while (*p)
    {
        if (isspace((unsigned char)*p))
        {
            // Skip multiple spaces
            if (p > start && !isspace((unsigned char)*(p - 1)))
            {
                *q++ = ' ';
            }
        }
        else
        {
            *q++ = *p;
        }
        p++;
    }
    *q = '\0';
}

void load_myshrc(char *homeDir)
{
    char path[MAX_COMMAND_LENGTH];
    snprintf(path, sizeof(path), "%s/.myshrc", homeDir);

    FILE *file = fopen(path, "r");
    if (!file)
    {
        perror("Error opening .myshrc");
        return;
    }

    char line[MAX_COMMAND_LENGTH];
    int inside_func = 0;
    int current_func_index = -1;

    while (fgets(line, sizeof(line), file))
    {
        trim_newline(line);

        // Handle comments
        char *comment_pos = strstr(line, "#") < strstr(line, "//") ? strstr(line, "#") : strstr(line, "//");
        if (comment_pos)
        {
            *comment_pos = '\0';
        }

        // Skip empty lines
        if (strlen(line) == 0)
        {
            continue;
        }

        // Handle aliases
        char *equals_pos = strchr(line, '=');
        if (equals_pos)
        {
            *equals_pos = '\0'; // Null-terminate the alias name
            char *alias_name = line + 6;
            char *command = equals_pos + 1;

            // Trim any leading or trailing spaces
            while (isspace((unsigned char)*alias_name))
                alias_name++;
            char *end = alias_name + strlen(alias_name) - 1;
            while (end > alias_name && isspace((unsigned char)*end))
                end--;
            *(end + 1) = '\0';

            while (isspace((unsigned char)*command))
                command++;
            end = command + strlen(command) - 1;
            while (end > command && isspace((unsigned char)*end))
                end--;
            *(end + 1) = '\0';

            if (alias_name[0] && command[0])
            {
                if (alias_count < MAX_ALIASES)
                {
                    strcpy(aliases[alias_count].alias_name, alias_name);
                    strcpy(aliases[alias_count].command, command);
                    alias_count++;
                }
                else
                {
                    fprintf(stderr, "Maximum number of aliases reached.\n");
                }
            }
        }

        // Handle function definitions
        else if (strstr(line, "func") && strstr(line, "()"))
        {
            char *func_name = strtok(line, " ");
            func_name = strtok(NULL, " \n()");

            if (func_name)
            {
                if (func_count < MAX_FUNCTIONS)
                {
                    strcpy(functions[func_count].func_name, func_name);
                    functions[func_count].num_lines = 0;
                    current_func_index = func_count;
                    func_count++;
                    inside_func = 1;
                }
                else
                {
                    fprintf(stderr, "Maximum number of functions reached.\n");
                }
            }
        }
        else if (strstr(line, "{"))
        {
            // Start of function body
            continue;
        }
        else if (strstr(line, "}"))
        {
            // End of function body
            inside_func = 0;
            current_func_index = -1;
        }
        else if (inside_func && current_func_index != -1)
        {
            // Handle lines inside function body
            if (functions[current_func_index].num_lines < MAX_BODY_LINES)
            {
                strcpy(functions[current_func_index].body[functions[current_func_index].num_lines], line);
                functions[current_func_index].num_lines++;
            }
            else
            {
                fprintf(stderr, "Function body exceeded maximum number of lines.\n");
            }
        }
    }

    fclose(file);

    for (int i = 0; i < func_count; i++)
    {
        for (int j = 0; j < functions[i].num_lines; j++)
        {
            trim_spaces(functions[i].body[j]);
            strcat(functions[i].body[j], "; ");
            strcat(functions[i].line, functions[i].body[j]);
        }
    }

    if (func_count || alias_count)
    {
        printf("Loaded custom shrc file\n");
    }
}

void print_loadedSHRC()
{
    printf("Alias: \n");
    for (int i = 0; i < alias_count; i++)
    {
        printf("%d) %s : %s\n", i + 1, aliases[i].alias_name, aliases[i].command);
    }

    printf("Functions: \n");
    for (int i = 0; i < func_count; i++)
    {
        printf("%d) %s\n", i + 1, functions[i].func_name);
        printf("\t %s\n", functions[i].line);
    }
}