#ifndef SLAVE_FUN_H
#define SLAVE_FUN_H

#include <complex.h>

// Generic
void err_exit(char *s);
void init_random();
double get_random_re();
double get_random_im();

// Fifo
void write_to_fifo(char *filename, int N, int K);

// Iters computation
int get_iters(double complex c, int K);

#endif