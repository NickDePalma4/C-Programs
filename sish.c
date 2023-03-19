#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parse.h"
#include "execute.h"

int cd(char *path) {
    return chdir(path);
}

int main(void) {
    char **input; 

    while(1) {
        input = tokenize(getInput());
        if (strcmp(input[0], "exit") == 0) {
            free(input);
            exit(EXIT_SUCCESS);
        } else if (strcmp(input[0], "cd") == 0) {
            if (cd(input[1]) < 0) {
                printf("cd: %s: No such file or directory", input[1]);
                free(input);
                exit(EXIT_FAILURE);
            }
        } else {
            execute(input);
        }
        free(input);
    } 
    
}

