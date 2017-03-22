#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "constrained_batch.h"

void print_usage(char *program_name);
int parse_arguments(char **argv, char **filename, long *virtual_mem_limit, long *proc_time_limit);

int main(int argc, char **argv)
{
    char *filename;
    long virtual_mem_limit;
    long proc_time_limit;
    if(argc != 4) {
        print_usage(argv[0]);
        return -1;
    }
    if(parse_arguments(argv, &filename, &virtual_mem_limit, &proc_time_limit) < 0) {
        fprintf(stderr, "%s\n", "Invalid argument");
        return -1;
    }
    process_batch_file(filename, proc_time_limit, virtual_mem_limit);
    return 0;
}

void print_usage(char *program_name)
{
    printf("Usage: %s %s\n", program_name, "<batch_filename> <proc_time_limit> <virtual_mem_limit>");
    printf("\nFilename must specify a batch file for the interpreter.\n");
    printf("\nProcessor time limit should be specified in seconds\n");
    printf("\nVirtual memory limit should be specified in megabytes\n");
}

int parse_arguments(char **argv, char **filename, long *virtual_mem_limit, long *proc_time_limit)
{
    if(access(argv[1], F_OK) < 0) {
        perror("Error");
        return -1;
    }
    if(access(argv[1], R_OK) < 0) {
        perror("Error");
        return -1;
    }
    *filename = argv[1];
    *proc_time_limit = strtol(argv[2], NULL, 10);
    if(errno == EINVAL || errno == ERANGE || proc_time_limit <= 0) {
        return -1;
    }
    *virtual_mem_limit = strtol(argv[3], NULL, 10);
    if(errno == EINVAL || errno == ERANGE || virtual_mem_limit <= 0) {
        return -1;
    }
    return 0;
}