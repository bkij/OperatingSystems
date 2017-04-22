#include "slave_fun.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>

void parse_args(char **argv, char **filename, int *N, int *K);

int main(int argc, char **argv)
{
    char *filename;
    int N;
    int K;
    if(argc < 4) {
        err_exit("Not enough arguments");
    }
    parse_args(argv, &filename, &N, &K);
    write_to_fifo(filename, N, K);
    exit(0);
}

void parse_args(char **argv, char **filename, int *N, int *K)
{
    *filename = argv[1];
    long tmp = strtol(argv[2], NULL, 10);
    if(tmp <= 0 || errno == ERANGE || tmp > INT_MAX) {
        err_exit("Invalid R argument");
    }
    *N = (int)tmp;
    tmp = strtol(argv[3], NULL, 10);    
    if(tmp <= 0 || errno == ERANGE || tmp > INT_MAX) {
        err_exit("Invalid R argument");
    }
    *K = (int)tmp;
}