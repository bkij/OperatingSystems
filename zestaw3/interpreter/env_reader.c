#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if(argc != 2) {
        fprintf(stderr, "%s\n", "No argument supplied.");
        exit(-1);
    }
    char *env_value = getenv(argv[1]);
    if(env_value != NULL) {
        printf("%s=%s\n", argv[1], env_value);
    }
    else {
        printf("%s environment variable not found.\n", argv[1]);
    }
    exit(0);
}