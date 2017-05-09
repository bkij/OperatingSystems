#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include "util.h"

void err_exit(const char *why)
{
    perror(why);
    exit(EXIT_FAILURE);
}

void print_timestamp()
{
    struct timespec time;
    if(clock_gettime(CLOCK_MONOTONIC, &time) < 0) {
        err_exit("Error getting timestamp");
    }
    printf("%lld.%.9ld: ", (long long)time.tv_sec, time.tv_nsec);
}