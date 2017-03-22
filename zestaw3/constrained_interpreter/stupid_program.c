#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>

int main()
{
    char *big = malloc(1024 * 1024 * 1024);

    FILE *file = fopen("/dev/null", "w");
    srand(time(NULL));

    int i = 0;
    while(1) {
        big[i % (1024 * 1024 * 1024)] = rand() % 100;
        i++;
        fwrite(big, 1, i % (1024 * 1024 * 1024), file);
    }
    return 0;
}