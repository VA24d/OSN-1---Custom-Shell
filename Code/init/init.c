#include "../base/base.h"

int main()
{
    clear_screen();

    struct termios term;

    // Get the terminal attributes
    tcgetattr(STDIN_FILENO, &term);
    // Modify the terminal attributes to enable cooked mode
    term.c_lflag |= (ICANON | ECHO);
    // Set the terminal attributes
    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    if (tcgetattr(STDIN_FILENO, &term) == -1)
    {
        perror("tcgetattr");
        return 1;
    }

    // Check if ICANON (cooked mode) is enabled
    if (term.c_lflag & ICANON)
    {
        printf(BOLD "Cooked mode\n" RESET);
    }
    else
    {
        printf(BOLD "Raw mode\n" RESET);
    }

    init_shell();

    printf(GREEN "Welcome to Compiler" BLUE BOLD " VA\n" RESET);
    printf(YELLOW "Please note that this program is only for educational purposes.\n" RESET);
    // printf(YELLOW "I am not responsible for any damage caused to any device.\n" RESET);
    printf(CYAN "***************************************************************\n" RESET);


    printf(MAGENTA "Author: Vijay A\n");
    printf(MAGENTA "Version: 1.0\n");
    printf(MAGENTA "Date: August 2024\n" RESET);

    // struct sigaction sa;

    // Set up signal handler for SIGINT
    // sa.sa_handler = handle_sigint;
    // sigemptyset(&sa.sa_mask);
    // sa.sa_flags = 0;
    // sigaction(SIGINT, &sa, NULL);

    signal(SIGINT, handle_sigint);
    signal(SIGTSTP, handle_sigtstp);

    int run = 1;
    int re = 0; // to handle prompt reprint
    int *p_run = &run;
    char input[in_size];
    char curDir[4096];
    char prevDir[4096];
    char homeDir[4096];
    char SystemName[256];

    char *UserName;
    if (getcwd(curDir, sizeof(curDir)) == NULL)
    {
        perror("getcwd() error");
        return 1;
    }

    strcpy(homeDir, curDir);
    strcpy(prevDir, curDir);

    if (gethostname(SystemName, sizeof(SystemName)) != 0)
    {
        perror("gethostname() error");
        return 1;
    }

    UserName = getenv("USER");
    if (UserName == NULL)
        printf("USER environment variable not set\n");

    initialize_log(homeDir);
    initialize_time(homeDir);
    load_myshrc(homeDir);
    printf("Shell ready\n");

    while (run)
    {
        // handle_background_processes();
        if (!re)
            print_prompt(UserName, SystemName, homeDir);

        if (fgets(input, in_size, stdin) == NULL)
        {
            // Handle EOF (Ctrl+D)
            handle_eof(p_run);
            break;
        }
        remove_newline(input);

        log_command(input, homeDir);

        // Parse and execute the commands
        parse_and_execute(input, curDir, prevDir, homeDir, p_run);

    }

    printf("Bye! (⁠╥⁠﹏⁠╥⁠)\n" BLUE BOLD "By Vijay 2024\n" RESET);
    return 0;
}