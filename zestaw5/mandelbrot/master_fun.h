#ifndef MASTER_FUN_H
#define MASTER_FUN_H

#include <errno.h>

typedef struct mandelbrot_point {
    int x;
    int y;
    int z;
} MandelbrotPoint;

// Generic
void err_exit(char *s);

// Fifo
void create_fifo(char *filename);
void read_fifo(char *filename, int **mandelbrot_table, int table_size);
void delete_fifo(char *filename);

// Table
int **get_table(int table_size);
void free_table(int **mandelbrot_table, int table_size);
void parse_mandelbrot_point(char *point_buffer, int table_size, MandelbrotPoint *point);


// Plot
void plot_mandelbrot(int **mandlebrot_table, int table_size);

#endif