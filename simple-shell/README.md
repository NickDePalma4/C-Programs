# Simple Shell

This project is a command-line interpreter or shell implementation written in C. The shell allows users to enter commands, which are then executed by creating child processes. After the execution of a command is finished, the shell prompts for more user input.

## Shell Executable

The shell executable is named `sish`, and the source code is located in `sish.c`.

## Usage

1. The shell runs continuously and displays a prompt when waiting for input. The prompt is exactly `sish> `, including the space after the greater than sign. For example:

         sish> ls -l
         

2. Commands entered by the user are parsed into the command and its arguments. The parsing is done by tokenizing the input line. Only whitespace characters (ASCII character number 32) are considered as delimiters. Special characters, such as quotation marks, backslashes, and tabs, are not handled. Therefore, the shell does not support arguments with spaces.

3. After parsing the command, the shell executes it. Executing an executable that is not a built-in command is done by invoking `fork()` followed by `execvp()`. The shell does not use the `system()` function.

## Built-in Commands

The shell supports the following built-in commands:

- `exit`: Exits the shell after performing any necessary cleanup.
- `cd [dir]`: Changes the current working directory of the shell. The `dir` argument specifies the directory to change to. Only a single command-line argument is accepted. The `chdir()` system call is used to change the directory. If `chdir()` fails, it is considered an error.
- `history [-c] [offset]`: Displays the last 100 commands the user ran, with an offset next to each command. The offset represents the index of the command in the list, ranging from 0 to 99. The oldest command has an offset of 0. Invalid commands and the `history` command itself should also appear in the list. If more than 100 commands are executed, the oldest entry is removed to accommodate newer commands.
- `history -c`: Clears the entire command history, removing all entries.
- `history [offset]`: Executes the command at the specified offset in the history list. If the offset is invalid, an error message is displayed.

## Pipes

The shell supports the execution of a sequence of programs that communicate through pipes. Multiple processes can communicate through pipes, not just two. To use pipes, the shell replaces the standard input and output file descriptors using `dup2()`.

Note: The shell does not provide built-in commands specifically designed to work with pipes.

## Disclaimer

Please note that the source code provided in this repository is available for viewing purposes only. It was originally developed as a project for a class, and while you are welcome to examine the code and learn from it, it should not be reused or submitted for class assignments or any other academic or commercial purposes. Using this code for such purposes would violate academic integrity policies and may have consequences.

## License

**Note:** The code in this repository is provided for the sole purpose of demonstration to potential employers. The code is not licensed for general use or distribution.

By accessing or using this code, you agree to the following terms:

1. You may only view and run the code for evaluation purposes related to employment opportunities.
2. You may not reproduce, distribute, modify, or use the code for any other purpose without explicit written permission.
3. You may not submit this code or any derivative of it for academic or educational purposes, including as part of coursework or assignments.

If you have any questions or need clarification on the usage terms, please contact me directly.


