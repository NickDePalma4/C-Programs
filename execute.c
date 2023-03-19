#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "execute.h"

void execute(char **argv) {
    pid_t pid;
    int stat;

    if ((pid = fork()) < 0) {
        printf("ERROR: Forking the process\n");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        if (execvp(*argv, argv) < 0) {
            printf("ERROR: Execution failed\n");
            exit(EXIT_FAILURE);
        }
    } else {
        while (wait(&stat) != pid);
    }
}