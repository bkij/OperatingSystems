#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <malloc.h>
#include <stdbool.h>
#include <errno.h>
#include "pipe.h"

#define READ_END 0
#define WRITE_END 1

void execute_pipe(char *command)
{
    int pipe_fds[MAX_COMMANDS][2];
    pid_t child_pid;
    Command current_command;
    bool last_command;

    signal(SIGPIPE, SIG_IGN);
    
    // Execute
    int i = 0;
    for(; i < MAX_COMMANDS; i++) {
        char *current_command_string = get_command_string(&command);
        last_command = (*command == '\0');
        if(current_command_string == NULL) {
            break;
        }
        parse_command(current_command_string, &current_command, command);
        printf("%s\n", current_command.command_name);
        for(int i = 0; current_command.command_args[i] != NULL; i++) {
            printf("%s ", current_command.command_args[i]);
        }
        printf("\n");
        if(pipe(pipe_fds[i]) < 0) {
            err_exit("pipe error");
        }
        child_pid = fork();
        if(child_pid < 0) {
            err_exit("fork error");
        }
        else if(child_pid == 0) {
            manage_pipeline(pipe_fds, i, last_command);
            if(execvp(current_command.command_name, current_command.command_args) < 0) {
                err_exit("execvp error");
            }
        }
        else {
            free(current_command_string);
            free_args(&current_command);
        }
    }
    for(int j = 0; j < i; j++) {
        // Close all pipes in parent as they're unused
        // So the children don't block
        // It can't be done in a loop because then child k wouldn't inherit k-1 write end
        close(pipe_fds[j][READ_END]);
        close(pipe_fds[j][WRITE_END]);
    }
    while(true) {
        if(wait(NULL) < 0 && errno == ECHILD) {
            break;
        }
    }
}

void manage_pipeline(int pipe_fd[MAX_COMMANDS][2], int idx, bool last_command)
{
    for(int i = 0; i < idx - 1; i++) {
        close(pipe_fd[i][READ_END]);
        close(pipe_fd[i][WRITE_END]);
    }
    if(idx == 0) {
        if(dup2(pipe_fd[idx][WRITE_END], STDOUT_FILENO) < 0) {
            err_exit("dup2 error");
        }
        close(pipe_fd[idx][WRITE_END]);
    }
    else if(last_command) {
        if(dup2(pipe_fd[idx - 1][READ_END], STDIN_FILENO) < 0) {
            err_exit("dup2 error");
        }
        close(pipe_fd[idx - 1][READ_END]);
    }
    else {
        if(dup2(pipe_fd[idx - 1][READ_END], STDIN_FILENO) < 0) {
            err_exit("dup2 error");
        }
        close(pipe_fd[idx - 1][READ_END]);
        if(dup2(pipe_fd[idx][WRITE_END], STDOUT_FILENO) < 0) {
            err_exit("dup2 error");
        }
        close(pipe_fd[idx][WRITE_END]);
    }
    if(idx > 0) {
        close(pipe_fd[idx - 1][WRITE_END]);
    }
    close(pipe_fd[idx][READ_END]);
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

void free_args(Command *command)
{
    free(command->command_name);
    for(int i = 0; command->command_args[i] != NULL; i++) {
        free(command->command_args[i]);
    }
}

void parse_command(char *current_command_string, Command *command, char *command_str)
{
    char *token = strtok(current_command_string, " ");
    command->command_name = malloc(strlen(token) + 1);
    strcpy(command->command_name, token);
    command->command_args[0] = malloc(strlen(token) + 1);
    strcpy(command->command_args[0], token);
    int idx = 1;
    for(token = strtok(NULL, " "); token != NULL; token = strtok(NULL, " ")) {
        command->command_args[idx] = malloc(strlen(token) + 1);
        strcpy(command->command_args[idx], token);
        idx++;
        if(idx == MAX_ARGS + 1) {
            break;
        }
    }
    command->command_args[idx] = NULL;
}

char *get_command_string(char **command)
{
    if(**command == '\0') {
        return NULL;
    }
    char *command_string;
    int idx = 0;
    while((*command)[idx] != '|' && (*command)[idx] != '\0') {
        idx++;
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