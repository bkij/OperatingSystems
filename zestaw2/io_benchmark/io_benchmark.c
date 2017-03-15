#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "file_manip.h"

bool parse_args(int argc, char **argv, int *num_records, int *record_size, char **filename, Mode *mode, FunctionType *type);
bool parse_record_options(char **argv, int *num_records, int *record_size, char **filename);
bool parse_mode_and_type(char **argv, Mode *mode, FunctionType *type);
void print_usage();

int main(int argc, char **argv)
{
    int num_records, record_size;
    char *filename;
    Mode mode;
    FunctionType type;
    return 0;
}

bool parse_args(int argc, char **argv, int *num_records, int *record_size, char **filename, Mode *mode, FunctionType *type)
{
    if(argc < 5) {
        print_usage();
        // TODO: Add perror
        return false;
    }
    if(!strcmp(argv[1], "generate")) {
        return parse_record_options(argv + 2, num_records, record_size, filename);
    }
    else {
        return parse_mode_and_type(argv, mode, type) && parse_record_options(argv + 3, num_records, record_size, filename);
    }
}

bool parse_record_options(char **argv, int *num_records, int *record_size, char **filename)
{
    if(argv[0] == NULL) {
        // TODO: Think if this is neccessary
        print_usage();
        return false;
    }
    *filename = argv[0];

    if(argv[1] == NULL) {
        print_usage();
        return false;
    }
    long tmp = strtol(argv[1], NULL, 10);
    if(errno == ERANGE || tmp <= 0 || tmp >= INT_MAX) {
        print_usage();
        return false;
    }
    *num_records = (int)tmp;

    tmp = strtol(argv[2], NULL, 10);
    if(errno == ERANGE || tmp <= 0 || tmp >= INT_MAX) {
        print_usage();
        return false;
    }
    *record_size = (int)tmp;

    return true;
}

void print_usage()
{
}
