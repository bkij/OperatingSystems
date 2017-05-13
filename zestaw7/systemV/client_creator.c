#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "util.h"
#include "shared.h"

void parse_args(char **argv, int *num_clients, int *num_haircuts);

int main(int argc, char **argv)
{
    int num_clients;
    int num_haircuts;
    if(argc < 3) {
        err_exit("Not enough arguments");
    }
    parse_args(argv, &num_clients, &num_haircuts);
    for(int i = 0; i < num_clients; i++) {
        pid_t pid = fork();
        if(pid < 0) {
            err_exit("Fork error");
        }
        else if (pid == 0) {
            if(execl("./client.out", "./client.out", (char *)&num_haircuts, (char *)NULL) < 0) {
                err_exit("execl error");
            }
        }
    }
    while(wait(NULL) > 0 && errno != ECHILD) {}
    exit(EXIT_SUCCESS);
}

void parse_args(char **argv, int *num_clients, int *num_haircuts)
{
    long tmp = strtol(argv[1], NULL, 10);
    if(tmp <= 0 || tmp > MAX_CLIENTS || errno == ERANGE) {
        err_exit("Invalid K argument (integer; min 1 max 256)");
    }
    *num_clients = (int)tmp;
    tmp = strtol(argv[2], NULL, 10);
    if(tmp <= 0 || tmp > MAX_HAIRCUTS || errno == ERANGE) {
        err_exit("Invalid S argument (integer; min 1 max 128)");
    }
    *num_haircuts = (int)tmp;
}