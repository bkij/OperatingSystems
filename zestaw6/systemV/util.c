#include <stdlib.h>
#include "util.h"

void err_exit(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}