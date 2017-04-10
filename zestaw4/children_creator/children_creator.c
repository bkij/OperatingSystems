#define _XOPEN_SOURCE 700
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <unistd.h>

const char child_filename[] = "child.out";

void print_usage_and_exit();
void parse_arguments(char **argv, int *N, int *M);
int validate_and_parse(char *string);
void create_children(int N, int M);

void set_sigusr1_handle();
void sigusr1_handle(int sigusr1, siginfo_t *signal_info, void *context);
void set_sigint_handle();
void sigint_handle(int sigint);
void set_rt_handles();
void rt_handle(int, siginfo_t *, void *);

volatile sig_atomic_t num_requests = 0;
volatile sig_atomic_t request_threshold;

// Children created
pid_t children_pids[1024];
volatile sig_atomic_t child_idx = 0;

int main(int argc, char **argv)
{
    int N;
    int M;
    if(argc < 3) {
        fprintf(stderr, "%s\n", "Not enough arguments");
        print_usage_and_exit();
    }
    parse_arguments(argv, &N, &M);
    request_threshold = M;
    signal(SIGTERM, SIG_IGN);   // Ignore as the process sends it to itself also
    set_sigint_handle();
    set_sigusr1_handle();
    set_rt_handles();
    create_children(N, M);
    exit(0);
}

void print_usage_and_exit()
{
    printf("%s %s %s %s\n", "Usage: ", __FILE__, "N", "M");
    printf("%s\n", "Options: N - number of child processes to create (max 1024)");
    printf("%s\n", "         M - number of requests to aggregate before accepting (max N)");
    exit(-1);
}

void parse_arguments(char **argv, int *N, int *M)
{
    *N = validate_and_parse(argv[1]);
    *M = validate_and_parse(argv[2]);
    if(*M > *N) {
        fprintf(stderr, "%s\n", "M bigger than N");
        print_usage_and_exit();
    }
}

int validate_and_parse(char *string)
{
    errno = 0;
    long tmp = strtol(string, NULL, 10);
    if(errno == ERANGE || tmp <= 0 || tmp > INT_MAX) {
        fprintf(stderr, "%s\n", "Error parsing N - value out of range or negative");
        print_usage_and_exit();
    }
    if(tmp > 1024) {
        fprintf(stderr, "%s\n", "Up to 1024 children supported");
        print_usage_and_exit();
    }
    return (int)tmp;
}

void create_children(int N, int M)
{
    int child_status;
    pid_t pid;
    pid_t child_pid;
    for(int i = 0; i < N; i++) {
        pid = fork();
        if(pid < 0) {
            perror("Fork error, aborting");
            exit(-1);
        }
        else if(pid == 0) {
            if(execl(child_filename, child_filename, (char *)NULL) < 0) {
                perror("Execl error, aborting");
                exit(-1);
            }
        }
    }
    // Wait for all children
    while(1) {
        child_pid = wait(&child_status);
        if(child_pid == -1 && errno == ECHILD) {
            break;
        }
        if(child_pid == -1 && errno == EINTR) {
            continue;
        }
        if(WIFEXITED(child_status)) {
            printf("Child with pid: %u exited with status %d\n", child_pid, WEXITSTATUS(child_status));
        }
        else if(WIFSIGNALED(child_status)) {
            printf("Child with pid: %u terminated by signal: %s\n", child_pid, strsignal(WTERMSIG(child_status)));
        }
    }
}

void set_rt_handles()
{
    struct sigaction action_def;
    sigset_t signals_blocked;
    if(sigfillset(&signals_blocked) < 0) {
        perror("Couldn't fill signal mask");
        exit(-1);
    }
    action_def.sa_sigaction = rt_handle;
    action_def.sa_mask = signals_blocked;
    action_def.sa_flags = SA_SIGINFO | SA_RESTART;

    for(int sig = SIGRTMIN; sig <= SIGRTMAX; sig++) {
        if(sigaction(sig, &action_def, NULL) < 0) {
            perror("Couldn't set signal action");
            exit(-1);
        }
    }
}

void set_sigusr1_handle()
{
    struct sigaction action_def;
    sigset_t signals_blocked;

    if(sigfillset(&signals_blocked) < 0) {
        perror("Couldn't fill signal mask");
        exit(-1);
    }

    action_def.sa_sigaction = sigusr1_handle;
    action_def.sa_mask = signals_blocked;
    action_def.sa_flags = SA_SIGINFO | SA_RESTART;

    if(sigaction(SIGUSR1, &action_def, NULL) < 0) {
        perror("Couldn't set signal action");
        exit(-1);
    }
}

void sigusr1_handle(int sigusr1, siginfo_t *signal_info, void *context)
{
    children_pids[child_idx] = signal_info->si_pid;
    child_idx++;
    num_requests++;
    if(num_requests == request_threshold) {
        for(int i = 0; i < child_idx; i++) {
            if(kill(children_pids[i], SIGCONT) < 0) {
                char *buf = "Error sending signal to child\n";
                write(STDERR_FILENO, buf, strlen(buf));
            }
        }
    }
    else if(num_requests > request_threshold) {
        if(kill(signal_info->si_pid, SIGCONT) < 0) {
            char *buf = "Error sending signal to child\n";
            write(STDERR_FILENO, buf, strlen(buf));
        }
    }
}

void rt_handle(int sigrt, siginfo_t *signal_info, void *context)
{
    printf("Got signal %s from child: %u\n", strsignal(sigrt), signal_info->si_pid);
}

void set_sigint_handle()
{
    struct sigaction action_def;
    sigset_t signals_blocked;

    if(sigfillset(&signals_blocked) < 0) {
        perror("Couldn't fill signal mask");
        exit(-1);
    }

    action_def.sa_handler = sigint_handle;
    action_def.sa_mask = signals_blocked;
    action_def.sa_flags = 0;

    if(sigaction(SIGINT, &action_def, NULL) < 0) {
        perror("Couldn't set signal action");
        exit(-1);
    }
}

void sigint_handle(int sigint)
{
    if(kill(0, SIGTERM) < 0) {
        char *buf = "Error sending kill signal to children\n";
        write(STDERR_FILENO, buf, strlen(buf));
    }
    // printf is unsafe here; probably TODO: fix
    pid_t child_pid;
    int child_status;
    while((child_pid = wait(&child_status)) >= 0) {
        if(WIFEXITED(child_status)) {
            printf("Child with pid: %u exited with status %d\n", child_pid, WEXITSTATUS(child_status));
        }
        else if(WIFSIGNALED(child_status)) {
            printf("Child with pid: %u terminated by signal: %s\n", child_pid, strsignal(WTERMSIG(child_status)));
        }
    }
}