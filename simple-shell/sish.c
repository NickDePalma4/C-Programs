#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>

char *history[100]; // char* array for storing commands useb by builtin_history
int count = 0; // Tracks the number of strings in the history array

// Used to handle the instance of a command being inserted into the array
// when it is full. Left shifts the array to make room at index 99, where 
// the value will be stored
void handleFull(char *command) {
    free(history[0]); // Clear index 0
    for (int i = 1; i < 100; i++) { // Left shift the array
        history[i - 1] = history[i];
    }
    history[99] = strdup(command); // Store the new command
}

// Adds the command the user entered into the history array
void addHistory(char *command) { 
    if (count < 100) { // Checks if history array is full
       history[count] = strdup(command); // If not store new command at index count
       count++; // Increase count by one
    } else { 
        handleFull(command); // If array is full call handleFull()
    }
}

// Gets user input
char* getInput() { 
    char *input = NULL;
    size_t length = 0;
    if (getline(&input, &length, stdin) == -1) { // Get user input
        printf("ERROR: Reading user input\n");
        exit(EXIT_FAILURE);
    }
    int modifier = strlen(input) - 1; 
    input[modifier] = '\0'; // Insert null terminator
    return input; // Return input string
}

// Adds spaces around pipe symbols in a string
char* spacePipe(const char* input) {
    size_t inputLength = strlen(input);
    char* modified = malloc((inputLength * 2 + 3) * sizeof(char)); // Allocate memory for modified string

    if (modified == NULL) { // malloc error
        printf("ERROR: Allocation failed\n");
        exit(EXIT_FAILURE);
    }

    size_t j = 0;

    for (size_t i = 0; i < inputLength; i++) {
        if (input[i] == '|') {
            modified[j++] = ' '; // Add space before pipe
            modified[j++] = '|'; // Add pipe
            modified[j++] = ' '; // Add space after pipe
        } else {
            modified[j++] = input[i]; // Copy other characters as-is
        }
    }

    modified[j] = '\0'; // Add null terminator

    return modified;
}

// Returns an array of tokenized strings that are tokenized by a space
char** tokenize(char* string) {
    char *input = string;
    char *delim = " ";
    char *token = "";
    char *saver = NULL;
    char **argList;
    int numArgs = 0; 

    if (strchr(input, '|') != NULL) {
        input = spacePipe(input); // If there is a pipe in the string, call spacePipe() to add spaces around it
    }

    for (int i = 0; input[i] != '\0'; i++) { // Find number of words in the string
        if ((input[i] == ' ' && input[i + 1] != ' ') || input[i] == '|' || input[i + 1] == '\0') {
            numArgs++; // Increases each time there is a space, pipe, or the string ends
        }
    }

    argList = malloc((numArgs + 2) * sizeof(char *)); // Allocate memory for return array
    
    if (argList == NULL) { // maloc error
        printf("ERROR: Allocation failed\n");
        exit(EXIT_FAILURE);
    }
    saver = input;
    
    for (int i = 0; i <= numArgs; i++) { // Calls strtok_r recursively to tokenize the user input
        token = strtok_r(saver, delim, &saver);
        argList[i] = token; // Stores token into the array
    }
    argList[numArgs + 1] = NULL; // Insert null terminator
    return argList; // Return array
}

// Used to fork the parent process so child process can execute commands
// it can also handle piped commands by properly forking child processes
// and piping accordingly
int launch(char** command) {
    pid_t pid;
    int stat;
    int numProcesses = 0;
    int length = 0;
    int i = 0; // Index of current process being tracked
    int j = 0; // Starting index of the current child process
    int k = 0; // Index of pipe symbol or null

    // Count the number of processes that need to be handled
    while (command[length] != NULL) {
        if (strcmp(command[length], "|") == 0) {
            numProcesses++;
        }
        length++;
    }

    int fd[numProcesses][2]; // Creates file descriptors for each process

    // Create pipes
    for (i = 0; i < numProcesses; i++) {
        if (pipe(fd[i]) < 0) {
            printf("ERROR: pipe\n");
            exit(EXIT_FAILURE);
        }
    }
    
    // Fork child process 
    for (i = 0, j = 0; i <= numProcesses; i++, j = k + 1) {
        k = j;
        while (command[k] != NULL && strcmp(command[k], "|") != 0) {
            k++;
        }
        char **args = malloc((k - j + 1) * sizeof(char *));
        for (int l = j; l < k; l++) {
            args[l - j] = command[l];
        }
        args[k - j] = NULL;
        if ((pid = fork()) < 0) {
            printf("ERROR: fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) { // Child process
            // Redirects the read pipe of first process in the array to standard input
            if (i > 0) { 
                if (dup2(fd[i - 1][0], STDIN_FILENO) < 0) {
                    printf("ERROR: dup2\n");
                    exit(EXIT_FAILURE);
                }
            }
            // Redirects the write pipe to standard output, going to the next process
            if (i < numProcesses) {
                if (dup2(fd[i][1], STDOUT_FILENO) < 0) {
                    printf("ERROR: dup2\n");
                    exit(EXIT_FAILURE);
                }
            }
            // Close pipes in child process
            for (int m = 0; m < numProcesses; m++) {
                close(fd[m][0]);
                close(fd[m][1]);
            }
            // Execute command
            if (execvp(*args, args) < 0) {
                printf("ERROR: invalid command\n");
                exit(EXIT_FAILURE);
            }    
        }
        free(args); // Free data args was taking up
    }
    // Close pipes in parent process
    for (i = 0; i < numProcesses; i++) {
        close(fd[i][0]);
        close(fd[i][1]);
    }
    // Parent process waits for all children
    while (wait(&stat) != pid);

    return 1;
}

// builtin command to change directory by using the chdir() system call
int builtin_cd(char **command) {
    if (command[1] == NULL) { // If no argument for the command print an error statement
        printf("%s: argument expected\n", command[0]);
    } else {
        if (chdir(command[1]) != 0) { // Call chdir() to change directory
            printf("%s: %s: No such file or directory\n", command[0], command[1]);
        }
    }
    return 1;
}

// builtin command to exit the program
int builtin_exit() {
    return 0; // Return 0 before forking to exit successfuly
}

// builtin command to handle history commands
int builtin_history(char **command) {
    if (command[1] == NULL) { // If there is no argument print the elements of the history array
        for (int i = 0; i < count; i++) {
        printf("%d %s\n", i, history[i]);
        }
    // If history -c is entered, clear the history array by  assigning all indexes to 0 and reseting count to 0
    } else if (strcmp(command[1], "-c") == 0) { 
        for (int i = 0; i < count; i++) {
            history[i] = 0;
        }
        count = 0;
    // Used for the history [offset] command
    } else {
        int length = strlen(command[1]); // Checks the length of the offset string
        char *validator = command[1]; // Used to check offset for certain parameters

        // Checks if all chars are digits, if not print an error message and return
        for (int i = 0; i < length; i++) {
            if (!isdigit(validator[i])) {
                printf("%s %s: arg2 must be an integer\n", command[0], validator);
                return 1;
            }
        }

        int offset = atoi(command[1]); // command[1] must be an integer, so var offset is assigned its value

        // Checks if offset is within the bounds of the history array, if it isn't print error message and return
        if (offset < 0 || offset > 99) {
            printf("%s %s: [offset] must be between values [0-99]\n", command[0], command[1]);
            return 1;
        // offset is within bounds and is a digit, so execute the command at that offset
        } else {
            char **executor = tokenize(history[offset]); // Tokenizes the history array entry
            if (executor[0] == NULL){ // Checks if command at history[0] isnt an empty string
                return 1;
            } else if (strcmp(executor[0], "history") == 0) { // If the command at index offset is a history call,                    
                builtin_history(executor);                    //recursively call the function
            } else { // If it is a normal command, call launch to execute it
                launch(executor); 
            }
        }
    }
    return 1;
}

// Handles various inputs to perform the appropriate function
int execute(char **command) {
    // Checks for empty string input
    if (command[0] == NULL) {
        return 1;
    } 

    if (strcmp(command[0], "cd") == 0) { // Execute builtin_cd if first index is cd
        return builtin_cd(command);
    } else if (strcmp(command[0], "exit") == 0) { // Execute builtin_exit if first index is exit
        return builtin_exit();
    } else if (strcmp(command[0], "history") == 0) { // execute builtin_history if first index is history
        return builtin_history(command);
    } else {
        return launch(command); // If it is a non builtin command, call launch
    }
}

// Used to start the execution process
void start() {
    char *input = NULL; // User input
    char **argList = NULL; // Tokenized input
    char *historyAdder = NULL; // Used to add history to the array
    int status; // Process status

    // Execute at least once
    do {
        printf("sish> "); // Print prompt
        input = getInput(); // Call getInput() to assign to input
        historyAdder = strdup(input); // Copy input to historyAdder
        argList = tokenize(input); // Call tokenize() to assign tokenized input array to argList
        status = execute(argList); // Assigns the process status to status that only returns 0 if exit is called
        addHistory(historyAdder); // Adds the input to the history array

        free(input); // Frees the memory input was taking up that was allocated in getInput()
        free(argList); // Frees the memory argList was taking up that was allocated in tokenize()
        free(historyAdder); // Frees the memory historyAdder was taking up that was allocated in strdup()
    } while (status); // Checks if exit has been called and stops the loop accordingly
}

// main method used to start program 
int main(void) {
    start(); // Calls start() to start execution process
    return EXIT_SUCCESS; // Return successfully after start is finished executing
}

