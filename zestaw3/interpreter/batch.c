#include <stdio.h>
#include <unistd.h>
#include "batch.h"

void process_batch_file(char *filename)
{
    char current_line[256];
    FILE *file = fopen(filename, "r");
    if(file == NULL) {
        perror("Error opening file");
        exit(-1);
    }
    while((current_line = fgets(current_line, sizeof(current_line), file)) != NULL) {
        execute(current_line);
    }
    if(ferror(file)) {
        perror("Error reading line from batch file");
        exit(-1);
    }
    exit(0);
}

void execute(char *line)
{
    pid_t pid;
    if(line[0] == '#') {
        // Change environment variable value
        char *var_name = get_env_variable(line);
        char *new_value = get_env_value(line);
        if(new_value == NULL) {
            delete_env_var(var_name);
            free(var_name);
        }
        else {
            change_env_var(var_name, new_value);
            free(var_name);
            free(new_value);
        }
    }
    else {
        // Execute command (program)
    }
}

char *get_env_value(char *line)
{
    int idx = 0;
    while(line[i] != ' ' && line[i] != '\0') {
        idx++;
    }
    if(line[i] == '\0') {
        return NULL;
    }
    int env_value_size = strlen(line + i);
    char *env_value = malloc(env_value_size); // Allocate memory for characters up to, not including, the newline character, and '\n'
    if(env_value == NULL) {
        perror("Malloc error");
        return NULL;
    }
    strncpy(env_value, (line + i), env_value_size - 1);
    env_value[env_value_size] = '\0';
    return env_value;
}