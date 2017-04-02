#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

const char *child_filename = "child.out";

void print_usage_and_exit();
void parse_arguments(char **argv, int *N, int *M);
int validate_and_parse(char *string);
void create_children(int N, int M);

volatile sig_atomic_t num_requests = 0;

int main(int argc, char **argv)
{
    int N;
    int M;
    if(argc < 3) {
        fprintf(stderr, "%s\n", "Not enough arguments");
        print_usage_and_exit();
    }
    parse_arguments(argv, &N, &M);
    create_children(N, M);
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

void create_children(int N, int M)
{
    pid_t pid;
    for(int i = 0; i < N; i++) {
        pid = fork();
        if(pid < 0) {
            perror("Fork error, aborting");
            exit(-1);
        }
        else if(pid == 0) {
            if(execl(child_filename, child_filename) < 0) {
                perror("Execl error, aborting");
                exit(-1);
            }
        }
        // Parent: go to next iteration
    }
    // TODO: Wait for children
}