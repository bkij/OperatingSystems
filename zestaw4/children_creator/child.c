#define _XOPEN_SOURCE
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void send_sigusr1_and_wait();
void send_random_rt_signal();
int get_time_elapsed(struct rusage *rusage_start);
void setup_signal_handle();
void handle_sigcont(int signum);


int main(int argc, char **argv)
{
    struct rusage rusage_start;
    sleep(10);
    if(getrusage(RUSAGE_SELF, &rusage_start) < 0) {
        perror("Couldn't get resource info");
        exit(-1);
    }
    send_sigusr1_and_wait();
    int time_elapsed = get_time_elapsed(&resource_info);
    printf("Child process %u: Time between sending request and sending RT signal (in seconds): %d\n", getpid(), time_elapsed);
    exit(time_elapsed);
}

void send_sigusr1_and_wait()
{
    if(kill(getppid(), SIGUSR1) < 0) {
        fprintf(stderr, "Child process %d couldn't send SIGUSR1 to parent\n", getpid());
        perror("Error: ");
        exit(-1)
    }
    setup_signal_handle();
    pause(); // Race condition! TODO: Fix
}

void setup_signal_handle()
{
    struct sigaction sig_info;
    sig_info.sa_handler = handle_sigcont;
    if(sigfillset(sig_info.sa_mask) < 0) {
        perror("Couldn't set blocking mask");
        exit(-1);
    }
    sig_info.sa_flags = 0;
    if(sigaction(SIGCONT, &sig_info, NULL) < 0) {
        perror("Couldn't set sigcont signal handler");
        exit(-1);
    }
}

int get_time_elapsed(struct rusage *rusage_start)
{
    struct rusage rusage_end;
    if(getrusage(RUSAGE_SELF, &rusage_end) < 0) {
        perror("Couldn't get resource info");
        exit(-1);
    }
    return rusage_end.tv_sec - rusage_start->tv_sec;
}

void handle_sigcont(int signum)
{
    int signum = mrand48() % 32;
    if(kill(getppid(), SIGRTMIN+signum) < 0) {
        fprintf(stderr, "Child process couldnt send SIGRTMIN+%d to parent\n", SIGRTMIN+signum);
        perror("Error");
        exit(-1);
    }
}