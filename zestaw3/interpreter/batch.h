#ifndef BATCH_H
#define BATCH_H

void process_batch_file(char *filename);
void execute(char *line);

char *get_env_value(char *line);
char *get_env_variable(char *line);

#endif