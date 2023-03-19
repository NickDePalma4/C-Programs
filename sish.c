#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parse.h"
#include "execute.h"

int main(void) {
    char **input; 

    while(1) {
        input = tokenize(getInput());
        if (strcmp(input[0], "exit") == 0) {
            exit(EXIT_SUCCESS);
        }
        execute(input);
    } 
}