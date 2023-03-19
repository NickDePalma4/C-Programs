#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"

char* getInput() {
    printf("sish> ");
    char *input = NULL;
    size_t length = 0;
    getline(&input, &length, stdin);
    int32_t modifier = strlen(input) - 1;
    input[modifier] = '\0';
    return input;
}

char** tokenize(char* string) {
    char *input = string;
    char *delim = " ";
    char *token = "";
    char *saver = NULL;
    char** argList;
    int numArgs = 0;

    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == ' ' && input[i + 1] != ' ') {
            numArgs++;
        }
    }

    printf("Number of arguments: %d\n", numArgs);

    argList = malloc((numArgs + 1) * sizeof(char *));
    saver = input;
    
    for (int i = 0; i < numArgs; i++) {
        token = strtok_r(saver, delim, &saver);
        argList[i] = token;
    }
    argList[numArgs] = NULL;
    return argList;
}