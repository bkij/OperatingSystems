#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include "file_manip.h"

bool parse_args(int argc, char **argv, int *num_records, int *record_size, char **filename, Command *command, FunctionType *type);
bool parse_record_options(char **argv, int *num_records, int *record_size, char **filename);
bool parse_command_and_type(char **argv, Command *command, FunctionType *type);
void print_usage(char *prog_name);

int main(int argc, char **argv)
{
    int num_records, record_size;
    char *filename;
    Command mode;
    FunctionType type;
    if(parse_args(argc, argv, &num_records, &record_size, &filename, &mode, &type) == false) {
        print_usage(argv[0]);
        //TODO: peror or something
        return -1;
    }
    return 0;
}

bool parse_args(int argc, char **argv, int *num_records, int *record_size, char **filename, Command *command, FunctionType *type)
{
    if(argc < 5) {
        return false;
    }
    if(!strcmp(argv[1], "generate")) {
        return parse_record_options(argv + 2, num_records, record_size, filename);
    }
    else {
        return parse_command_and_type(argv + 1, command, type) && parse_record_options(argv + 3, num_records, record_size, filename);
    }
}

bool parse_record_options(char **argv, int *num_records, int *record_size, char **filename)
{
    *filename = argv[0];
    
    long tmp = strtol(argv[1], NULL, 10);
    if(errno == ERANGE || tmp <= 0 || tmp >= INT_MAX) {
        return false;
    }
    *num_records = (int)tmp;

    // Argv[2] can be NULL when 5 arguments are given and the command isnt shuffle or sort
    if(argv[2] == NULL) {
        return false;
    }
    tmp = strtol(argv[2], NULL, 10);
    if(errno == ERANGE || tmp <= 0 || tmp >= INT_MAX) {
        return false;
    }
    *record_size = (int)tmp;

    return true;
}

bool parse_command_and_type(char **argv, Command *command, FunctionType *type)
{
   if(!strcmp(argv[0], "sys")) {
       *type = sys;
   }
   else if(!strcmp(argv[0], "lib")) {
       *type= lib;
   }
   else {
       return false;
   }
   if(!strcmp(argv[1], "shuffle")) {
       *command = shuffle;
   }
   else if(!strcmp(argv[1], "sort")) {
       *command = sort;
   }
   else {
       return false;
   }
   return true;
}

void print_usage(char *prog_name)
{
    printf("\n%s\n\n", "IO_BENCHMARK");
    printf("Usage: %s [generate] [filename] [num_records] [record_size]\n", prog_name);
    printf("   or: %s [sys/lib] [shuffle/sort] [filename] [num_records] [record_size]\n", prog_name);
    printf("\n");
    printf("%s\n",
           "Options: (order matters, as above)");
    printf("%s\n%s\n",
           "  generate:     creates a file filled with random bytes, containing",
           "                $num_records records, each of size $record_size    ");
    printf("%s\n",
           "  shuffle:      shuffle records in the file $filename randomly     ");
    printf("%s\n%s\n",
           "  sort:         sort the records in the file by comparing the first",
           "                byte of each record                                ");
    printf("%s\n",
           "  sys/lib:      use kernel or library functions respectively       ");
    printf("%s\n",
           "  filename:     work on the file with given filename               ");
    printf("%s\n",
           "  num_records:  specify the number of records in the file          ");
    printf("%s\n",
           "  record_size:  specify the size of a single record in bytes       ");
}
