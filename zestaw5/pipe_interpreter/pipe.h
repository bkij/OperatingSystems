#ifndef PIPE_H
#define PIPE_H

#include <stdbool.h>

#define MAX_ARGS 3
#define MAX_COMMANDS 20    // Unneeded?

typedef struct command {
    char *command_name;
    char *command_args[1 + MAX_ARGS + 1];   // Program name, MAX_ARGS arguments and NULL.
} Command;

void free_args(Command *command);

void execute_pipe(char *command);
void parse_command(char *current_command_string, Command *command, char *command_str);
void strip_potential_newline(char *command);
char *get_command_string(char **command);

void create_pipe(int pipe_fd[MAX_COMMANDS][2], int idx);
void manage_pipeline(int pipe_fd[MAX_COMMANDS][2], int idx, bool last_command);

void close_fds(int pipe_fds[MAX_COMMANDS][2], int idx);
void err_exit(char *s);

#endif