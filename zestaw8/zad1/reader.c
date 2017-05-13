#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "reader_fun.h"
#include "util.h"

void parse_args(char **argv, int *num_threads, char *filename, int *num_records, char *pattern, int *type);

int main(int argc, char **argv)
{
    int num_threads;
    char *filename;
    int num_records;
    char *pattern;
    int type;
    if(argc < 6) {
        fprintf(stderr, "%s\n", "Not enough arguments");
        exit(EXIT_FAILURE);
    }
    parse_args(argv, &num_threads, filename, &num_records, pattern, &type);
    dispatch_threads(num_threads, filename, num_records, pattern, type);
    exit(EXIT_SUCCESS);
}

void parse_args(char **argv, int *num_threads, char *filename, int *num_records, char *pattern, int *type)
{
    *num_threads = parse_int(argv[1], 1, MAX_THREADS);
    strcpy(filename, argv[2]);
    *num_records = parse_int(argv[3], 1, MAX_RECORDS);
    strcpy(pattern, argv[4]);
    *type = parse_int(argv[5], 1, 3);
}