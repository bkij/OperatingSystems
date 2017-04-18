#ifndef MASTER_FUN_H
#define MASTER_FUN_H

// Generic
void err_exit(char *s);

// Fifo
void create_fifo(char *filename);
void read_fifo(char *filename, double **mandelbrot_table);

// Table
int **get_table(int table_size);
void free_table(double **mandelbrot_table, int table_size);

// Plot
void plot_mandelbrot(double **mandlebrot_table);

#endif