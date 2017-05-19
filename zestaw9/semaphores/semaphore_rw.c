#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "sem_commons/sem_commons.h"
#include "sem_execution/sem_execution.h"

//
// Created by kveld on 19.05.17.
//

void parse_args(int argc, char *argv[], int *num_readers, int *num_writers, bool *verbose);

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


void parse_args(int argc, char *argv[], int *num_readers, int *num_writers, bool *verbose)
{
    if(argc < 4) {
        fprintf(stderr, "%s\n", "Usage: ./semaphore_rw.out NUM_READERS NUM_WRITERS VERBOSE");
    }
    *num_readers = atoi(argv[1]);
    *num_writers = atoi(argv[2]);
    *verbose = atoi(argv[3]) == 1 ? true : false;
}
