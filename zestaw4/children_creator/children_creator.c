#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>

void print_usage_and_exit();
void parse_arguments(char **argv, int *N, int *M);
int validate_and_parse(char *string);

int main(int argc, char **argv)
{
    int N;
    int M;
    if(argc < 3) {
        fprintf(stderr, "%s\n", "Not enough arguments");
        print_usage_and_exit();
    }
    parse_arguments(argv, &N, &M);
    exit(0);
}

void print_usage_and_exit()
{
    printf("%s %s %s %s\n", "Usage: ", __FILE__, "N", "M");
    printf("%s\n", "Options: N - number of child processes to create");
    printf("%s\n", "         M - number of requests to aggregate before accepting")
    exit(-1);
}

void parse_arguments(char **argv, int *N, int *M)
{
    *N = validate_and_parse(argv[0]);
    *M = validate_and_parse(argv[1]);
}

int validate_and_parse(char *string)
{
    errno = 0;
    long tmp = strtol(string, NULL, 10);
    if(errno == ERANGE || tmp < 0 || tmp > INT_MAX) {
        fprintf(stderr, "%s\n", "Error parsing N - value out of range or negative");
        print_usage_and_exit();
    }
    return (int)tmp;
}