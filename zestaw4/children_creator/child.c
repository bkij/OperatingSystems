#define _XOPEN_SOURCE 700
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


void send_sigusr1_and_wait();
void send_random_rt_signal();
int get_time_elapsed(struct rusage *rusage_start);
void setup_signal_handle();
void handle_sigcont(int sigcont);
int get_random_seed();

static jmp_buf after_signal;

int main(int argc, char **argv)
{
    struct timeval before;
    struct timeval after;
    srand48(get_random_seed());
    sleep(lrand48() % 10);
    gettimeofday(&before, NULL);
    if(setjmp(after_signal) == 0) {
        send_sigusr1_and_wait();
    }
    gettimeofday(&after, NULL);
    long time_elapsed = after.tv_sec - before.tv_sec;
    printf("Child process %u: Time between sending SIGUSR request and sending RT signal (in seconds): %ld\n", getpid(), time_elapsed);
    exit(time_elapsed);
}

void send_sigusr1_and_wait()
{
    if(kill(getppid(), SIGUSR1) < 0) {
        fprintf(stderr, "Child process %d couldn't send SIGUSR1 to parent\n", getpid());
        perror("Error: ");
        exit(-1);
    }
    setup_signal_handle();
    pause();
}

void setup_signal_handle()
{
    struct sigaction sig_info;
    sig_info.sa_handler = handle_sigcont;
    if(sigfillset(&(sig_info.sa_mask)) < 0) {
        perror("Couldn't set blocking mask");
        exit(-1);
    }
    if(sigdelset(&(sig_info.sa_mask), SIGTERM) < 0) {
        perror("Couldn't set blocking mask");
        exit(-1);
    }
    sig_info.sa_flags = SA_RESTART;
    if(sigaction(SIGCONT, &sig_info, NULL) < 0) {
        perror("Couldn't set sigcont signal handler");
        exit(-1);
    }
}

void handle_sigcont(int sigcont)
{
    int rt_signum = lrand48() % 31;
    if(kill(getppid(), SIGRTMIN+rt_signum) < 0) {
        fprintf(stderr, "Child process couldnt send SIGRTMIN+%d to parent\n", SIGRTMIN+rt_signum);
        perror("Error");
        exit(-1);
    }
    longjmp(after_signal, 1);
}

int get_random_seed()
{
    char byte_buf[4];
    int rand_fd = open("/dev/urandom", O_RDONLY);
    if(rand_fd < 0) {
        perror("Error opening /dev/urandom");
        exit(-1);
    }
    if(read(rand_fd, byte_buf, 4) < 0) {
        perror("Reading from /dev/urandom returned an error");
        exit(-1);
    }
    close(rand_fd);
    return (int)*byte_buf;
}