#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void send_sigusr1_and_wait();
void send_random_rt_signal();
void print_time_elapsed();

int main(int argc, char **argv)
{
    sleep(10);
    send_sigusr1_and_wait();
    send_random_rt_signal();
    print_time_elapsed();
    exit(0);
}