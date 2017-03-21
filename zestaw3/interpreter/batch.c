#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "batch.h"


void process_batch_file(char *filename)
{
    char current_line[256];
    FILE *file = fopen(filename, "r");
    if(file == NULL) {
        perror("Error opening file");
        exit(-1);
    }
    while(fgets(current_line, sizeof(current_line), file) != NULL) {
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
        // Execute command
        char *arguments[256];
        char *command;

        parse_command(line, &command, arguments);
        pid_t pid = fork();
        if(pid < 0) {
            perror("Fork error");
            exit(-1);
        }
        else if(pid == 0) {     // Child
            if(execvp(command, arguments) < 0) {
                perror("Execve error");
                exit(-1);
            }
        }
        else {                  // Parent
            int status;
            wait(&status);
            if(WIFEXITED(status)) {
                if(WEXITSTATUS(status) < 0) {
                    fprintf(stderr, "Error: %s returned %d. Aborting\n", command, WEXITSTATUS(status));
                    exit(-1);
                }
            }
            else if(WIFSIGNALED(status)) {
                fprintf(stderr, "%s\n", "Current command executing process terminated by a signal. Aborting...");
                exit(-1);
            }
        }
    }
}

void parse_command(char *line ,char **command, char **arguments)
{
    char *token;
    *command = strtok(line, " ");
    while((token = strtok(NULL, " ")) != NULL) {
        *arguments = token;
        arguments++;
    }
    *arguments = NULL;
}

void delete_env_var(char *env_variable)
{
    if(unsetenv(env_variable) < 0) {
        perror("Couldn't delete environment variable");
    }
}

void change_env_var(char *env_variable, char *new_value)
{
    if(setenv(env_variable, new_value, 1) < 0) {
        perror("Couldn't set environment variable");
    }
}

char *get_env_variable(char *line)
{
    int idx = 0;
    while(line[idx] != ' ' && line[idx] != '\0') {
        idx++;
    }
    char *env_variable = malloc(idx);
    strncpy(env_variable, line, idx - 1);
    env_variable[idx - 1] = '\0';
    return env_variable;
}

char *get_env_value(char *line)
{
    int idx = 0;
    while(line[idx] != ' ' && line[idx] != '\0') {
        idx++;
    }
    if(line[idx] == '\0') {
        return NULL;
    }
    int env_value_size = strlen(line + idx);
    char *env_value = malloc(env_value_size); // Allocate memory for characters up to, not including, the newline character, and '\n'
    if(env_value == NULL) {
        perror("Malloc error");
        return NULL;
    }
    strncpy(env_value, (line + idx), env_value_size - 1);
    env_value[env_value_size] = '\0';
    return env_value;
}