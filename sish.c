#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

char *historyBuff[100];
int count = 0;

char* getInput() {
    char *input = NULL;
    size_t length = 0;
    if (getline(&input, &length, stdin) == -1) {
        printf("ERROR: Reading user input\n");
        exit(EXIT_FAILURE);
    }
    int modifier = strlen(input) - 1;
    input[modifier] = '\0';
    return input;
}

char** tokenize(char* string) {
    char *input = string;
    char *delim = " ";
    char *token = "";
    char *saver = NULL;
    char **argList;
    int numArgs = 0;

    for (int i = 0; input[i] != '\0'; i++) {
        if ((input[i] == ' ' && input[i + 1] != ' ')|| input[i + 1] == '\0') {
            numArgs++;
        }
    }

    argList = malloc((numArgs + 2) * sizeof(char *));
    
    if (argList == NULL) {
        printf("ERROR: Allocation failed\n");
        exit(EXIT_FAILURE);
    }
    saver = input;
    
    for (int i = 0; i <= numArgs; i++) {
        token = strtok_r(saver, delim, &saver);
        argList[i] = token;
    }
    argList[numArgs + 1] = NULL;
    return argList;
}

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
            if (i > 0) {
                if (dup2(fd[i - 1][0], STDIN_FILENO) < 0) {
                    printf("ERROR: dup2\n");
                    exit(EXIT_FAILURE);
                }
            }
            if (i < numProcesses) {
                if (dup2(fd[i][1], STDOUT_FILENO) < 0) {
                    printf("ERROR: dup2\n");
                    exit(EXIT_FAILURE);
                }
            }
            for (int m = 0; m < numProcesses; m++) {
                close(fd[m][0]);
                close(fd[m][1]);
            }
            if (execvp(*args, args) < 0) {
                printf("ERROR: invalid command\n");
                exit(EXIT_FAILURE);
            }    
        }
        free(args);
    }
    
    for (i = 0; i < numProcesses; i++) {
        close(fd[i][0]);
        close(fd[i][1]);
    }
    while (wait(&stat) != pid);

    return 1;
}

void handleFull(char *command) {
    for (int i = 0; i < 99; i++) {
        historyBuff[i] = strdup(historyBuff[i + 1]);
    }
    historyBuff[99] = strdup(command);
}

void addHistory(char *command) {
    if (count < 100) {
       historyBuff[count] = strdup(command);
       count++;
    } else {
        handleFull(command);
    }
}

int builtin_cd(char **command) {
    if (command[1] == NULL) {
        printf("%s: argument expected\n", command[0]);
    } else {
        if (chdir(command[1]) != 0) {
            printf("%s: %s: No such file or directory\n", command[0], command[1]);
        }
    }
    return 1;
}

int builtin_exit() {
    return 0;
}

int builtin_history(char **command) {
    if (command[1] == NULL) {
        for (int i = 0; i < count; i++) {
        printf("%d %s\n", i, historyBuff[i]);
        }
    } else if (strcmp(command[1], "-c") == 0) {
        for (int i = 0; i < count; i++) {
            historyBuff[i] = 0;
        }
        count = 0;
    } else {
        int length = strlen(command[1]);
        char *validator = command[1];

        for (int i = 0; i < length; i++) {
            if (!isdigit(validator[i])) {
                printf("%s %s: arg2 must be an integer\n", command[0], validator);
                return 1;
            }
        }

        int offset = atoi(command[1]);

        if (offset < 0 || offset > 99) {
            printf("%s %s: [offset] must be between values [0-99]\n", command[0], command[1]);
            return 1;
        } else {
            char **executor = tokenize(historyBuff[offset]);
            if (executor[0] == NULL){
                return 1;
            } else if (strcmp(executor[0], "history") == 0) {
                builtin_history(executor);
            } else {
                launch(executor);
            }
        }
    }
    return 1;
}

int execute(char **command) {
    if (command[0] == NULL) {
        return 1;
    } 

    if (strcmp(command[0], "cd") == 0) {
        return builtin_cd(command);
    } else if (strcmp(command[0], "exit") == 0) {
        return builtin_exit();
    } else if (strcmp(command[0], "history") == 0) {
        return builtin_history(command);
    } else {
        return launch(command);
    }
}

void start() {
    char *input = NULL;
    char **argList = NULL;
    int status;

    do {
        printf("sish> ");
        input = getInput();
        addHistory(input);
        argList = tokenize(input);
        status = execute(argList);

        free(input);
        free(argList);
    } while (status);
}

int main(void) {
    start();
    return EXIT_SUCCESS;
}

