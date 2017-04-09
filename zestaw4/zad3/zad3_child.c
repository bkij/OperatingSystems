#define  _POSIX_C_SOURCE 210000L    //SA_RESTART
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <setjmp.h>

void unblock_signals(char *type);
void send_signals(char *type);
void print_info();

void sigusr1_handler(int sigusr1);
void sigrtmin_handler(int sigrtmin);
void terminating_handler(int signum);

void setup_handles(char *type);
void setup_rt_handling();
void setup_sigusr_handling();

void err_exit(char *s);

volatile sig_atomic_t signals_gotten = 0;
volatile sig_atomic_t terminating_gotten = 0;

int main(int argc, char **argv)
{
    setup_handles(argv[1]);
    unblock_signals(argv[1]);
    while(!terminating_gotten) {}
    send_signals(argv[1]);
    print_info();
    exit(0);
}

void err_exit(char *s)
{
    perror(s);
    exit(-1);
}

void setup_rt_handling()
{
    struct sigaction sig_info;
    sigset_t signals_blocked;
    
    // Setup SIGRTMIN handle
    if(sigfillset(&signals_blocked) < 0) {
        err_exit("sigfillset error in child");
    }
    if(sigdelset(&signals_blocked, SIGRTMIN) < 0) {
        err_exit("sigdelset error in child");
    }
    sig_info.sa_flags = SA_RESTART | SA_NODEFER;
    sig_info.sa_handler = sigrtmin_handler;

    if(sigaction(SIGRTMIN, &sig_info, NULL) < 0) {
        err_exit("sigaction error in child");
    }

    // Setup SIGRTMIN+1 handle
    sig_info.sa_handler = terminating_handler;

    if(sigaction(SIGRTMIN+1, &sig_info, NULL) < 0) {
        err_exit("sigaction error in child");
    }
}

void setup_sigusr_handling()
{
    struct sigaction sig_info;
    sigset_t signals_blocked;

    // Setup SIGUSR1 handle
    if(sigfillset(&signals_blocked) < 0) {
        err_exit("sigfillset error in child");
    }
    if(sigdelset(&signals_blocked, SIGUSR1) < 0) {
        err_exit("sigdelset error in child");
    }
    sig_info.sa_flags = SA_RESTART | SA_NODEFER;
    sig_info.sa_handler = sigusr1_handler;

    if(sigaction(SIGUSR1, &sig_info, NULL) < 0) {
        err_exit("sigaction error in child");
    }

    // Setup SIGUSR2 handle
    sig_info.sa_handler = terminating_handler;

    if(sigaction(SIGUSR2, &sig_info, NULL) < 0) {
        err_exit("sigaction error in child");
    }

}

void setup_handles(char *type)
{
    if(*type == 3) {
        setup_rt_handling();
    }
    else {
        setup_sigusr_handling();
    }
}

void sigusr1_handler(int sigusr1)
{
    signals_gotten++;
}

void sigrtmin_handler(int sigrtmin)
{
    signals_gotten++;
}

void terminating_handler(int signum)
{
    terminating_gotten = 1;
}

void print_info()
{
    printf("Child received %d signals\n", signals_gotten + 1);  // Terminating signal wasn't counted in signals_gotten
}

void unblock_signals(char *type)
{
    sigset_t unblocked_set;
    if(sigemptyset(&unblocked_set) < 0) {
        err_exit("sigemptyset error");
    }
    if(sigaddset(&unblocked_set, SIGTERM) < 0) {
        err_exit("sigaddset error");
        // SIGTERM unblocked in case parent receives SIGINT
    }
    if(*type != 3) {
        if(sigaddset(&unblocked_set, SIGUSR1) < 0) {
            err_exit("sigaddset error");
        }
        if(sigaddset(&unblocked_set, SIGUSR2) < 0) {
            err_exit("sigaddset error");
        }
    }
    else {
        if(sigaddset(&unblocked_set, SIGRTMIN) < 0) {
            err_exit("sigaddset error");;
        }
        if(sigaddset(&unblocked_set, SIGRTMIN+1) < 0) {
            err_exit("sigaddset error");
        }
    }
    if(sigprocmask(SIG_UNBLOCK, &unblocked_set, NULL) < 0) {
        err_exit("sigprocmask error");
    }
}

void send_signals(char *type)
{
    if(*type == 3) {
        for(int i = 0; i < signals_gotten; i++) {
            if(kill(getppid(), SIGRTMIN) < 0) {
                err_exit("kill error");
            }
        }
    }
    else {
        for(int i = 0; i < signals_gotten; i++) {
            if(kill(getppid(), SIGUSR1) < 0) {
                err_exit("kill error");
            }
        }
    }
}