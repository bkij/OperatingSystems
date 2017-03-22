#ifndef BATCH_H
#define BATCH_H

void process_batch_file(char *filename);
void strip_potential_newline(char *line);
void execute(char *line);
void wait_for_child(char *command);
void parse_command(char *line, char **command, char **arguments);
void delete_env_var(char *env_variable);
void change_env_var(char *env_variable, char *new_value);


char *get_env_value(char *line);
char *get_env_variable(char *line);

#endif