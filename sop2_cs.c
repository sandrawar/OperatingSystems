#define _GNU_SOURCE
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#define ERR(source)\
(fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

volatile sig_atomic_t last_signal = SIGUSR1;

void usage(char* name)
{
    fprintf(stderr, "usage: %s p>0, t>0, list of probabilities>0", name);
    exit(EXIT_FAILURE);
}

void set_handler(void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
    if(-1 == sigaction(sigNo, &act, NULL))
    {
        ERR("sigaction:");
    }
}

void chld_handler(int sigNo)
{
    last_signal = sigNo;
}

void set_parent_handler(void (*f)(int, siginfo_t*, void*), int sigNo)
{
    struct sigaction act;
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = f;
}

void parent_handler(int sigNo, siginfo_t* info, void* contex)
{
    kill(info->si_pid, SIGUSR2);
}

int child_work(int p, int t, int no, int prob)
{
    fprintf(stdout, "STUDENT [%d,%d] has started doing task!, prob:%d\n", no, getpid(), prob);
    int issues = 0;
    srand(time(NULL)*getpid());
    struct timespec tt = {0, t * 100000000};
    struct timespec ts = {0, 50000000};
    struct timespec th = {0, 10000000};
    for(int i = 1; i <= p; i++)
    {
        printf("Student [%d, %d] is starting doing part %d of %d!\n", no, getpid(), i, p);
        nanosleep(&tt, NULL);
        if(rand()%(100+1) < prob)
        {
            printf("Student [%d, %d] has issue (%d) doing task!\n", no, getpid(), ++issues);
            nanosleep(&ts, NULL);
        }
        printf("Student [%d, %d] has finished doing part %d of %d!\n", no, getpid(), i, p);
        while(last_signal != SIGUSR2)
        {
            kill(getppid(), SIGUSR1);
            nanosleep(&th, NULL);
        }
    }
    return issues;
}

int main(int argc, char** argv)
{
    if (argc < 3)
        usage(argv[0]);
    int p = atoi(argv[1]);
    int t = atoi(argv[2]);
    set_parent_handler(parent_handler, SIGUSR1);
    for(int i = 3; i < argc; i++)
    {
        pid_t s = fork();
        if (s < 0)
            ERR("fork:");
        if (s == 0)
        {
            set_handler(chld_handler, SIGUSR2);
            child_work(p, t, i-2, atoi(argv[i]));
        }
        if (s > 0)
           ;//parent_work();
    }
    while(wait(NULL) > 0)
    {
        ;
    }
    return EXIT_SUCCESS;
}
