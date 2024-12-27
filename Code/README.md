
# mini-project-1-template

## How to Run

- Navigate to the base of the repo. (ls output should contain ```Makefile``` and `init.c`)
- Open the terminal and type ```make```
- This will compile all the files into an executable named ```shell```
- To run this exectable, run ```./shell``` in the terminal.
- To delete the complied program, run ```make clean```.
- Details on how to use the shell, can be accessed be typing help

## Assumptions

### [Spec 2] input

- Complete functionality
- max input size > 4096

### [Spec 3] hop

- Complete functionality
- initial previous directory is home directory

### [Spec 4] reveal

- Complete functionality
- Color: 
  files: default
  folder: blue
  executable: green
  hidden: purple

### [Spec 5] log

- complete functionality
- Entire line stored
- no error checking before adding to log
- command added to log not alias

### [Spec 6] System Commands

- Complete functionality (As far as I know)
- sleep 4 ; sleep 5; sleep 6
  will only have sleep 6 in prompt as its the last command
- Not handling BG for custom functions
- ```handle quotes for commands by not splitting```
  - ```enables git commit with message```

- max BG = 4096

### [Spec 7] proclore

- 

### [Spec 8] seek

- Complete functionality

### [Spec 9] .myshrc

- Complete functionality

### [Spec 10] I/O Redirection

- Complete functionality
- Any file size input (works with large input)

### [Spec 11] Pipes

- Complete functionality

### [Spec 12] Redirection with Pipes

- Complete functionality

### [Spec 13] Activities

- Complete (Mostly)

### [Spec 14] Signals

- Complete (As far as i checked)
  - Ctrl + Z in a re-fg exits everything

### [Spec 15] Background and Foreground Processes

- fg is a bit buggy (randomly doesn't work)

### [Spec 16] Neonate

- Complete functionality

### [Spec 17] iMan

- Complete + html custom parse

## File structure

- **`init/init.c`**:
  - Initializes the shell environment, setting up signal handling, parsing input commands, and executing them within the shell loop.
  - No header file needed as this is the main entry point of the program.
- ```.command_history.log``` (Hidden)
  - Stores a list of commands used in the shell
- ```.time.txt``` (Hidden)
  - Stores the previous command if run time for command > 3
- ```.myshrc``` (Hidden)
  - Stores custom aliases and functions
  - can be modified
- **Images**
  - Images for chatgpt prompts
- **`base`**:
  - `base.c`: Contains core functionalities and helper functions that are shared across different modules in the shell.
  - `base.h`: Header file for `base.c`, declaring shared functions and constants.

- **`hop`**:
  - `hop.c`: Implements the `hop` command, which allows changing the current working directory with support for special cases like `.` (current directory), `..` (parent directory), `~` (home directory), and `-` (previous directory).
  - `hop.h`: Header file for `hop.c`, declaring functions related to directory navigation.

- **`reveal`**:
  - `reveal.c`: Implements the `reveal` command, which lists files and directories with color coding for different types (executables, directories, hidden files, etc.).
  - `reveal.h`: Header file for `reveal.c`, declaring functions related to file listing and color coding.

- **`log`**:
  - `log.c`: Manages logging functionalities, possibly including time logging for commands that take a significant amount of time to execute.
  - `log.h`: Header file for `log.c`, declaring functions related to logging.

- **`proclore`**:
  - `proclore.c`: Implements the `proclore` command, which displays information about processes, such as PID, status, memory usage, and executable path.
  - `proclore.h`: Header file for `proclore.c`, declaring functions related to process information.

- **`syscomm`**:
  - `syscomm.c`: Provides system command execution functionalities, including the handling of background and foreground processes.
  - `syscomm.h`: Header file for `syscomm.c`, declaring functions for executing system commands.

- **`seek`**:
  - `seek.c`: Implements the `seek` command, which searches for files or directories based on specified search criteria and flags.
  - `seek.h`: Header file for `seek.c`, declaring functions related to file search.

---

## References

- [Link](https://stackoverflow.com/questions/8953424/how-to-get-the-username-in-c-c-in-linux)
- [Link](https://www.unix.com/programming/21041-getting-username-c-program-unix.html)
- man pages
- way too many websites

## Notices

- Erroneous commands are also stored in the log.
- The background handler is called only after next input is taken.
  - Same as ```zsh```
- Coded on mac (some commands might mess up🥲)
- INPUTS are limited to 4096 characters
