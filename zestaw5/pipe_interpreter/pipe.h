#ifndef PIPE_H
#define PIPE_H

#define MAX_ARGS 3
#define MAX_COMMANDS 20    // Unneeded?

typedef struct command {
    char *command_name;
    char *command_args[MAX_ARGS + 1];   // MAX_ARGS arguments and NULL
    int pipe_fd[2];
} Command;

void free_command(Command *command);

void execute_pipe(char *command, Command *parent_command);
void parse_command(char *current_command_string, Command *command, Command *parent_command, char *command_str);
void strip_potential_newline(char *command);
char *get_command_string(char **command);


void err_exit(char *s);

#endif