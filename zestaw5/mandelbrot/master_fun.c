#define _XOPEN_SOURCE
#include "master_fun.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define RW_USER 0600

// Generic
void err_exit(char *s)
{
    perror(s);
    exit(-1);
}

void strip_potential_newline(char *buffer)
{
    for(int i = 0; buffer[i] != '\0'; i++) {
        if(buffer[i] == '\n') {
            buffer[i] = ' ';
            break;
        }
    }
}

// Fifo
void create_fifo(char *filename)
{
    if(mkfifo(filename, RW_USER) < 0) {
        err_exit("Error creating fifo");
    }
}

void read_fifo(char *filename, int **mandelbrot_table, int table_size)
{
    FILE *fifo = fopen(filename, "r");
    MandelbrotPoint point;
    char point_buffer[256];
    if(fifo == NULL) {
        err_exit("Error opening FIFO");
    }
    
    while(fgets(point_buffer, sizeof(point_buffer), fifo) != NULL) {
        parse_mandelbrot_point(point_buffer, table_size, &point);
        mandelbrot_table[point.x][point.y] = point.z;
    }

    if(!feof(fifo)) {
        fclose(fifo);
        err_exit("Error while reading FIFO");
    }
    fclose(fifo);
}

void delete_fifo(char *filename)
{
    if(unlink(filename) < 0) {
        err_exit("Couldn't unlink fifo file");
    }
}

// Table
int **get_table(int table_size)
{
    int **table;
    table = malloc(sizeof(int *) * table_size);
    for(int i = 0; i < table_size; i++) {
        table[i] = calloc(table_size, sizeof(int));
    }
    return table;
}

void free_table(int **mandelbrot_table, int table_size)
{
    for(int i = 0; i < table_size; i++) {
        free(mandelbrot_table[i]);
    }
    free(mandelbrot_table);
}

void parse_mandelbrot_point(char *point_buffer, int table_size, MandelbrotPoint *point)
{
    // Parse string
    char *token;
    strip_potential_newline(point_buffer);
    token = strtok(point_buffer, " ");
    double x = strtod(token, NULL);
    token = strtok(NULL, " ");
    double y = strtod(token, NULL);
    token = strtok(NULL, " ");
    int z = strtol(token, NULL, 10);
    
    // Get array values
    x += 2.0;
    point->x = (int)(x * table_size / 3.0);
    if(point->x == table_size) {
        point->x -= 1;
    }

    y += 1.0;
    point->y = (int)(y * table_size / 2.0);
    if(point->x == table_size) {
        point->y -= 1;
    }

    point->z = z;
}

// Plot
void plot_mandelbrot(int **mandelbrot_table, int table_size)
{
    FILE *gnuplot_input_data = fopen("data", "w");
    char *format_string = "%d %d %d\n";
    if(gnuplot_input_data == NULL) {
        err_exit("Couldn't create data file");
    }
    // Write to file
    for(int i = 0; i < table_size; i++) {
        for(int j = 0; j < table_size; j++) {
            if(fprintf(gnuplot_input_data, format_string, i, j, mandelbrot_table[i][j]) < 0) {
                if(ferror(gnuplot_input_data)) {
                    err_exit("Error writing to data file");
                }
            }
        }
    }
    fclose(gnuplot_input_data);

    // Send commands to gnuplot
    FILE *gnuplot_pipe = popen("gnuplot", "w");
    fprintf(gnuplot_pipe, "%s\n", "set view map");
    fprintf(gnuplot_pipe, "set xrange [0:%d]\n", table_size);
    fprintf(gnuplot_pipe, "set yrange [0:%d]\n", table_size);
    fprintf(gnuplot_pipe, "%s\n", "plot 'data' with image");
    fflush(gnuplot_pipe);
    getc(stdin);

    pclose(gnuplot_pipe);
    if(unlink("data") < 0) {
        fprintf(stderr, "%s\n", "Couldnt unlink data");
    }
}