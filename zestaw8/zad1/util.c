#include <errno.h>
#include <stdlib.h>
#include "util.h"

void errno_exit(char *why)
{
    perror(why);
    exit(EXIT_FAILURE);
}