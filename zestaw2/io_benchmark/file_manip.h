#ifndef FILE_MANIP_H
#define FILE_MANIP_H

typedef enum {do_generate, do_shuffle, do_sort} Command;
typedef enum {sys, lib} FunctionType;

int execute(char *filename, int num_records, int record_size, Command command, FunctionType type);
int generate(char *filename, int num_records, int record_size, FunctionType type);
int shuffle(char *filename, int num_records, int record_size, FunctionType type);
int sort(char *filename, int num_records, int record_size, FunctionType type);

#endif
