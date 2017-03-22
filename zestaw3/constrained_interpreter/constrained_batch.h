#ifndef BATCH_H
#define BATCH_H

#include <sys/resource.h>

void process_batch_file(char *filename, long proc_time_limit, long virtual_mem_limit);
void strip_potential_newline(char *line);
void execute(char *line, long proc_time_limit, long virtual_mem_limit);
void wait_for_child(char *command);
void parse_command(char *line, char **command, char **arguments);
void delete_env_var(char *env_variable);
void change_env_var(char *env_variable, char *new_value);
void set_limits(long proc_time_limit, long virtual_mem_limit);
void print_child_usage(struct rusage *before, struct rusage *current);


char *get_env_value(char *line);
char *get_env_variable(char *line);

#endif