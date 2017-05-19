//
// Created by kveld on 18.05.17.
//
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdbool.h>
#include "commons/commons.h"
#include "execution/execution.h"

void parse_args(int argc, char **argv, int *num_readers, int *num_writers, bool *verbose);

int main(int argc, char *argv[])
{
    int shared_array[SHARED_ARR_LEN];
    int num_readers;
    int num_writers;
    bool verbose = false;
    parse_args(argc, argv, &num_readers, &num_writers, &verbose);
    execute_threads(shared_array, num_readers, num_writers, verbose);
    exit(EXIT_SUCCESS);
}


void parse_args(int argc, char **argv, int *num_readers, int *num_writers, bool *verbose)
{
    if(argc < 4) {
        fprintf(stderr, "%s\n", "Usage: ./mutex_rw.out NUM_READERS NUM_WRITERS VERBOSE");
        exit(-1);
    }
    *num_readers = atoi(argv[1]);
    *num_writers = atoi(argv[2]);
    *verbose = atoi(argv[3]) == 1 ? true: false;
}