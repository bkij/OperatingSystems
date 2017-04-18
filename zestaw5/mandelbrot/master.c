#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "master_fun.h"

void parse_args(char **argv, char **filename, int *table_size);

int main(int argc, char **argv)
{
    char *filename;
    int table_size;
    FILE *fifo;
    double **mandelbrot_table;
    if(argc < 3) {
        err_exit("Not enough arguments");
    }
    parse_args(argv, &filename, &table_size);
    mandelbrot_table = get_table(table_size);
    create_fifo(filename);
    read_fifo(filename, mandelbrot_table);    // Possible sleep inside
    plot_mandelbrot(mandelbrot_table);
    free_table(mandelbrot_table);
    exit(0);
}

void parse_args(char **argv, char **filename, int *table_size)
{
    *filename = argv[1];
    long tmp = strtol(argv[2], NULL, 10);
    if(tmp <= 0 || errno == ERANGE || tmp > INT_MAX) {
        err_exit("Invalid R argument");
    }
    *table_size = (int)tmp;
}