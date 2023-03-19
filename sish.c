#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"

int main(void) {
    char **input = tokenize(getInput());
    int i = 0;
    while (input[i] != NULL) {
        printf("%s\n", input[i]);
        i++;
    }
    if (input[i] == NULL) {
        printf("Null pointer exists\n");
    }
    return 0;
}