#include "master_fun.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <complex.h>

#define RW_USER 0600

// Generic
void err_exit(char *s)
{
    perror(s);
    exit(-1);
}

// Fifo
void *create_fifo(char *filename)
{
    if(mkfifo(filename, RW_USER) < 0) {
        err_exit("Error creating fifo");
    }
}

void read_fifo(char *filename, double **mandelbrot_table)
{
    FILE *fifo = fopen(filename, "r");
    if(fifo == NULL) {
        err_exit("Error opening FIFO");
    }
    
}

// Table
int **get_table(int table_size)
{
    double **table;
    table = malloc(sizeof(double *) * table_size);
    for(int i = 0; i < table_size; i++) {
        table[i] = calloc(sizeof(double) * table_size);
    }
    return table;
}

void free_table(double **mandelbrot_table, int table_size)
{
    for(int i = 0; i < table_size; i++) {
        free(mandelbrot_table[i]);
    }
    free(mandelbrot_table);
}