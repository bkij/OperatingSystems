#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <malloc.h>
#include "pipe.h"

#define READ_END 0
#define WRITE_END 1

void execute_pipe(char *command, Command *parent_command)
{
    pid_t child_pid;
    Command *current_command = malloc(sizeof(Command));
    char *current_command_string = get_command_string(&command);
    if(current_command_string == NULL) {
        return;
    }
    parse_command(current_command_string, current_command, parent_command, command);
   
    // Execute
    child_pid = fork();
    if(child_pid < 0) {
        err_exit("Fork error");
    }
    if(child_pid == 0) {
        execute_pipe(command, current_command);
        if(execvp(current_command->command_name, current_command->command_args) < 0) {
            err_exit("execvp error");
        }
    }
    if(child_pid > 0) {
        // Cleanup and wait
        if(parent_command == NULL) {
            close(current_command->pipe_fd[READ_END]);
        }
        else if(*command == '\0') {
            close(parent_command->pipe_fd[WRITE_END]);
        }
        else {
            close(parent_command->pipe_fd[WRITE_END]);
            close(current_command->pipe_fd[READ_END]);
        }
        free(current_command_string);
        wait(NULL);
        free_command(current_command);
    }
    
}

void strip_potential_newline(char *command)
{
    for(int i = 0; command[i] != '\0'; i++) {
        if(command[i] == '\n') {
            command[i] = '\0';
            break;
        }
    }
}

void err_exit(char *s)
{
    perror(s);
    exit(-1);
}


void parse_command(char *current_command_string, Command *command, Command *parent_command, char *command_str)
{
    char *token = strtok(current_command_string, " ");
    command->command_name = token;
    int idx = 0;
    for(token = strtok(NULL, " "); token != NULL; token = strtok(NULL, " ")) {
        command->command_args[idx] = malloc(strlen(token) + 1);
        strcpy(command->command_args[idx], token);
        idx++;
        if(idx == MAX_ARGS) {
            break;
        }
    }
    command->command_args[idx] = NULL;
    if(pipe(command->pipe_fd) < 0) {
        err_exit("pipe error");
    }
    if(parent_command == NULL) {
        if(dup2(command->pipe_fd[WRITE_END], STDOUT_FILENO) < 0) {
            err_exit("dup2 error");
        }
    }
    else if(*command_str == '\0') {
        if(dup2(parent_command->pipe_fd[READ_END], STDIN_FILENO) < 0) {
            err_exit("dup2 error");
        }
    }
    else {
        if(dup2(parent_command->pipe_fd[READ_END], STDIN_FILENO) < 0) {
            err_exit("dup2 error");
        }
        if(dup2(command->pipe_fd[WRITE_END], STDOUT_FILENO) < 0) {
            err_exit("dup2 error");
        }
    }
}

char *get_command_string(char **command)
{
    char *command_string;
    int idx = 0;
    while((*command)[idx] != '|' && (*command)[idx] != '\0') {
        idx++;
    }
    if(idx == 0) {
        // Hit '\0'
        return NULL;
    }

    if((*command)[idx] == '\0') {
        command_string = malloc(idx + 1);
        strncpy(command_string, *command, idx + 1);
    }
    else {
        command_string = malloc(idx);
        strncpy(command_string, *command, idx);
        command_string[idx - 1] = '\0';
    }

    *command = *command + idx;
    if((*command)[0] == '|') {
        *command += 2;  // Assuming space after pipe
    }

    return command_string;
}

void free_command(Command *command)
{
    for(int i = 0; i < MAX_ARGS + 1; i++) {
        if(command->command_args[i] == NULL) {
            break;
        }
        free(command->command_args[i]);
    }
    free(command);
}