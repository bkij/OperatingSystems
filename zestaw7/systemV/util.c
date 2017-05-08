#include <stdlib.h>
#include <errno.h>
#include "util.h"

void err_exit(const char *why)
{
    perror(why);
    exit(EXIT_FAILURE);
}