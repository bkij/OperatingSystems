#define _XOPEN_SOURCE 
#include "slave_fun.h"
#include <complex.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>

// Generic
void err_exit(char *s)
{
    perror(s);
    exit(-1);
}

void init_random()
{
    char buf[4];
    FILE *rng = fopen("/dev/urandom", "r");
    if(fread(buf, sizeof(buf), sizeof(char), rng) == 0) {
        if(ferror(rng)) {
            err_exit("Error initalizing rng");
        }
    }
    srand48(*((int *)&buf));
}

double get_random_re()
{
    return (drand48() * 3.0) - 2.0;
}

double get_random_im()
{
    return (drand48() * 2.0) - 1.0;
}

// Fifo
void write_to_fifo(char *filename, int N, int K)
{
    FILE *fifo = fopen(filename, "w");
    if(fifo == NULL) {
        err_exit("Couldn't open fifo");
    }
    for(int i = 0; i < N; i++) {
        double complex random_point = get_random_re() + get_random_im() * I;
        int iters = get_iters(random_point, K);
        fprintf(fifo, "%lf %lf %d\n", creal(random_point), cimag(random_point), iters);
    }
    fclose(fifo);
}

// Iters computation
int get_iters(double complex c, int K)
{
    double complex z = 0.0 + 0.0 * I;
    int i = 0;
    while(i < K) {
        i++;
        z = z*z + c;
        if(cabs(z) > 2.0) {
            break;
        }
    }
    return i;
}

