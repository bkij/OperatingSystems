#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "constrained_batch.h"

void process_batch_file(char *filename, long proc_time_limit, long virtual_mem_limit)
{
    char current_line[256];
    FILE *file = fopen(filename, "r");
    if(file == NULL) {
        perror("Error opening file");
        exit(-1);
    }
    while(fgets(current_line, sizeof(current_line), file) != NULL) {
        strip_potential_newline(current_line);
        execute(current_line, proc_time_limit, virtual_mem_limit);
    }
    if(ferror(file)) {
        perror("Error reading line from batch file");
        exit(-1);
    }
    exit(0);
}

void execute(char *line, long proc_time_limit, long virtual_mem_limit)
{
    struct rusage before = {0};
    struct rusage current;
    if(line[0] == '#') {
        // Change environment variable value
        // + 1 to get the string without hash
        char *var_name = get_env_variable(line + 1);
        char *new_value = get_env_value(line + 1);
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
            set_limits(proc_time_limit, virtual_mem_limit);
            if(execvp(command, arguments) < 0) {
                perror("Execve error");
                exit(-1);
            }
        }
        else {                  // Parent
            wait_for_child(command);
            if(getrusage(RUSAGE_CHILDREN, &current) < 0) {
                perror("Error getting child resource usage");
                exit(-1);
            }
            print_child_usage(&before, &current);
        }
    }
}

void wait_for_child(char *command)
{
    int status;
    wait(&status);
    if(WIFEXITED(status)) {
        if(WEXITSTATUS(status) != 0) {
            fprintf(stderr, "\nError: %s returned %d. Aborting...\n", command, WEXITSTATUS(status));
            exit(-1);
        }
    }
    else if(WIFSIGNALED(status)) {
        fprintf(stderr, "Process executing command %s terminated by signal %s. Aborting...\n",
            command, strsignal(WTERMSIG(status)));
        exit(-1);
    }
}

void strip_potential_newline(char *line)
{
    for(int i = 0; line[i] != '\0'; i++) {
        if(line[i] == '\n') {
            line[i] = '\0';
        }
    }
}

void parse_command(char *line ,char **command, char **arguments)
{
    char *token = strtok(line, " ");
    *command = token;
    *arguments++ = token;
    while((token = strtok(NULL, " ")) != NULL) {
        *arguments++ = token;
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
    char *env_variable = malloc(idx + 1);
    strncpy(env_variable, line, idx);
    env_variable[idx] = '\0';
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
    int env_len = strlen(line + idx + 1);
    char *env_value = malloc(env_len + 1);
    if(env_value == NULL) {
        perror("Malloc error");
        return NULL;
    }
    strncpy(env_value, (line + idx + 1), env_len);
    env_value[env_len] = '\0';
    return env_value;
}

void set_limits(long proc_time_limit, long virtual_mem_limit)
{
    struct rlimit proc_limit;
    struct rlimit mem_limit;
    if(getrlimit(RLIMIT_CPU, &proc_limit) < 0) {
        perror("Error getting current cpu time limit");
        exit(-1);
    }
    if(getrlimit(RLIMIT_MEMLOCK, &mem_limit) < 0) {
        perror("Error getting current memory limit");
        exit(-1);
    }

    proc_limit.rlim_max = proc_time_limit;
    if(proc_limit.rlim_cur > proc_limit.rlim_max) {
        proc_limit.rlim_cur = proc_limit.rlim_max;
    }
    mem_limit.rlim_max = virtual_mem_limit * 1024 * 1024;
    if(mem_limit.rlim_cur > mem_limit.rlim_max) {
        mem_limit.rlim_cur = mem_limit.rlim_max;
    }
    
    if(setrlimit(RLIMIT_CPU, &proc_limit) < 0) {
        perror("Error setting cpu time limit");
        exit(-1);
    }
    if(setrlimit(RLIMIT_MEMLOCK, &mem_limit) < 0) {
        perror("Error setting memory limit");
        exit(-1);
    }
}

void print_child_usage(struct rusage *before, struct rusage *current)
{
    printf("%s\n", "Time elapsed in microseconds:");
    printf("\tUser: %lu\n",
        ((current->ru_utime.tv_sec - before->ru_utime.tv_sec) * 1000000L) + current->ru_utime.tv_usec - before->ru_utime.tv_usec);
    printf("\tSystem: %lu\n", 
        ((current->ru_stime.tv_sec - before->ru_stime.tv_sec) * 1000000L) + current->ru_stime.tv_usec - before->ru_stime.tv_usec);
    *before = *current;
}