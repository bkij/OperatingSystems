#include <unistd.h>
#include <stdio.h>
#include "batch.h"

void print_usage(char *program_name);
void parse_arguments(char **argv, char **filename);

int main(int argc, char **argv)
{
    char *filename;
    if(argc != 2) {
        print_usage(argv[0]);
        return -1;
    }
    if(parse_arguments(argv, &filename) < 0) {
        fprintf(stderr, "%s\n", "Invalid file argument");
        return -1;
    }
    process_batch_file(filename);
    return 0;
}

void print_usage(char *program_name)
{
    printf("\nUsage: %s %s\n", program_name, "<batch_filename>");
    printf("\nFilename must specify a batch file for the interpreter.\n");
}

int parse_arguments(char **argv, char **filename)
{
    if(access(argv[1], F_OK) < 0) {
        perror("Error");
        return -1;
    }
    if(access(argv[1], R_OK | W_OK | X_OK) < 0) {
        perror("Error");
        return -1;
    }
    *filename = argv[1];
    return 0;
}