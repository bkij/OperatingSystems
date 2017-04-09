#define _XOPEN_SOURCE
#define _POSIX_C_SOURCE 210000L    //SA_RESTART
#include <stdio.h>
#include <signal.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

void parse_args(int *L, int *type, char **argv);

void setup_handlers(int type);
sigset_t block_all_temp();
void unblock_all(sigset_t old);
void signal_handler(int signal);
void kill_child_exit(int sigint);

void send_signals(pid_t child_pid, int L, int type);

pid_t create_child();

void wait_and_print_info();

const char child_filename[] = "zad3_child.out";

volatile sig_atomic_t signals_sent = 0;
volatile sig_atomic_t signals_gotten = 0;

int main(int argc, char **argv)
{
    pid_t child_pid;
    int L, type;
    if(argc < 3) {
        fprintf(stderr, "%s\n", "Not enough arguments");
        exit(-1);
    }
    parse_args(&L, &type, argv);
    setup_handlers(type);
    child_pid = create_child(type);
    send_signals(child_pid, L, type);
    wait_and_print_info();
    exit(0);
}

void parse_args(int *L, int *type, char **argv)
{
    long tmp;
    tmp = strtol(argv[1], NULL, 10);
    if(errno == ERANGE || tmp <= 0 || tmp > INT_MAX) {
        fprintf(stderr, "%s\n", "Error parsing L");
        exit(-1);
    }
    *L = (int)tmp;

    tmp = strtol(argv[2], NULL, 10);
    if(errno == ERANGE || tmp < 1 || tmp > 3) {
        fprintf(stderr, "%s\n", "Error parsing Type");
        exit(-1);
    }
    *type = (int)tmp;
}

void setup_handlers(int type)
{
    struct sigaction sig_info;
    if(sigfillset(&(sig_info.sa_mask)) < 0) {
        perror("Couldn't set signal mask");
        exit(-1);
    }
    if(sigdelset(&(sig_info.sa_mask), SIGINT) < 0) {
        perror("Couldn't set signal mask");
        exit(-1);
    }
    sig_info.sa_flags = SA_RESTART | SA_NODEFER;
    sig_info.sa_handler = signal_handler;


    if(type == 3) {
        if(sigdelset(&(sig_info.sa_mask), SIGRTMIN) < 0) {
            perror("Couldn't set signal mask");
            exit(-1);
        }
        if(sigaction(SIGRTMIN, &sig_info, NULL) < 0) {
            perror("Couldn't set signal action");
            exit(-1);
        }
    }
    else {
        if(sigdelset(&(sig_info.sa_mask), SIGUSR1) < 0) {
            perror("Couldn't set signal mask");
            exit(-1);
        }
        if(sigaction(SIGUSR1, &sig_info, NULL) < 0) {
            perror("Couldn't set signal action");
            exit(-1);
        }
    }

    signal(SIGINT, kill_child_exit);
}

sigset_t block_all_temp()
{
    sigset_t old;
    sigset_t blocked;
    if(sigfillset(&blocked) < 0) {
        perror("sigfillset error");
        exit(-1);
    }
    if(sigprocmask(SIG_BLOCK, &blocked, &old) < 0) {
        perror("sigprocmask error");
        exit(-1);
    }
    return old;
}
void unblock_all(sigset_t old)
{
    if(sigprocmask(SIG_SETMASK, &old, NULL) < 0) {
        perror("sigsetmask error");
        exit(-1);
    }
}

void signal_handler(int sigusr1)
{
    signals_gotten++;
}

void kill_child_exit(int sigint)
{
    signal(SIGTERM, SIG_IGN);
    if(kill(0, SIGTERM) < 0) {
        perror("Error sending shutdown signal to child");
        exit(-1);
    }
    wait(NULL);
    exit(0);
}

pid_t create_child(int type)
{
    sigset_t oldmask = block_all_temp();
    pid_t pid = fork();
    if(pid < 0) {
        perror("Fork error");
        exit(-1);
    }
    else if(pid > 0) {
        unblock_all(oldmask);
        return pid;
    }
    else {
        char type_arg = (int)type;
        if(execl(child_filename, child_filename, (char *)&type_arg, (char *)NULL) < 0) {
            perror("execl error");
            exit(-1);
        }
    }
}

void send_signals(pid_t child_pid, int L, int type)
{
    union sigval added_value;
    for(int i = 0; i < L; i++) {
        switch(type) {
            case 1:
                if(kill(child_pid, SIGUSR1) < 0) {
                    perror("Error sending sigusr1 to child");
                    exit(-1);
                }
                break;
            case 2:
                if(sigqueue(child_pid, SIGUSR1, added_value) < 0) {
                    perror("Error sending sigusr1 to child");
                    exit(-1);
                }
                break;
            case 3:
                if(kill(child_pid, SIGRTMIN) < 0) {
                    perror("Error sending sigrtmin to child");
                    exit(-1);
                }
                break;
            default:
                fprintf(stderr, "%s\n", "ERROR");
                break;
        }
        signals_sent++;
    }
    switch(type) {
            case 1:
                if(kill(child_pid, SIGUSR2) < 0) {
                    perror("Error sending sigusr2 to child");
                    exit(-1);
                }
                break;
            case 2:
                if(sigqueue(child_pid, SIGUSR2, added_value) < 0) {
                    perror("Error sending sigusr2 to child");
                    exit(-1);
                }
                break;
            case 3:
                if(kill(child_pid, SIGRTMIN+1) < 0) {
                    perror("Error sending sigrtmin+1 to child");
                    exit(-1);
                }
                break;
            default:
                fprintf(stderr, "%s\n", "ERROR");
                break;
        }
}

void wait_and_print_info()
{
    int child_status;
    pid_t child_pid;
    do {
        child_pid = wait(&child_status);
    } while(child_pid > 0 && errno != ECHILD);
    printf("Signals sent to child: %d\n", signals_sent + 1); // +1 because terminating signal isn't counted in signals sent
    printf("Signals gotten from child: %d\n", signals_gotten);
}