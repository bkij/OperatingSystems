#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>

bool increment = true;

void write_alphabet();
void set_sigtstp_handle();

// Handlers
void die_on_sigint(int sigint);
void reverse_on_sigtstp(int sigtstp);

int main(void)
{
    set_sigtstp_handle();
    write_alphabet();
    exit(0);
}

void write_alphabet()
{
    char current = 'A';
    while(true) {
        printf("%c\n", current);
        if(increment) {
            current++;
            if(current == 91) {
                current = 'A';
            }
        }
        else {
            current--;
            if(current == 64) {
                current = 'Z';
            }
        }
        sleep(1);
    }
}

void die_on_sigint(int sigint)
{
    if(sigint != SIGINT) {
        fprintf("%s\n", "FATAL ERROR, SIGINT HANDLER DIDNT GET SIGINT");
        _exit(-1);
    }
    printf("%s\n", "Odebrano sygnal SIGINT");
    _exit(0);
}

void set_sigtstp_handle()
{
    struct sigaction action_def;
    sigset_t signals_blocked;

    if(sigfillset(&signals_blocked) < 0) {
        perror("Couldn't fill signal mask");
        exit(-1)
    }
    if(sigdelset(&signals_blocked, SIGTSTP) < 0) {
        perror("Couldn't unblock SIGTSTP");
        exit(-1);
    }

    action_def.sa_handler = reverse_on_sigtstp;
    action_def.sa_mask = signals_blocked;
    action_def.sa_flags = 0;

    if(sigaction(SIGTSTP, &action_def, NULL) < 0) {
        perror("Couldn't set signal action");
        exit(-1);
    }
}

void reverse_on_sigtstp(int sigtstp)
{
    if(sigtstp != SIGTSTP) {
        fprintf("%s\n", "FATAL ERROR, SIGTSTP HANDLER DIDNT GET SIGTSTP");
        _exit(-1);
    }
    increment = !increment;
}