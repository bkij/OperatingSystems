#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "pipe.h"

int main()
{
    char input_buffer[1024];
    while(fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
        execute_pipe(input_buffer);
    }
    if(!feof(stdin)) {
        perror("Error using fgets");
        exit(-1);
    }
    exit(0);
}