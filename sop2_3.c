#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#define ERR(source)\
(fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

volatile sig_atomic_t last_signal = 0;

void usage(char* name)
{
    fprintf(stderr, "usage: %s m p", name);
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

void sig_handler(int sig)
{
    last_signal = sig;
}

void sigchld_handler(int sig)
{
    pid_t pid;
    for(;;)
    {
        pid = waitpid(0,  NULL, WNOHANG);
        if(pid == 0)
            return;
        if (pid <= 0)
        {
            if(errno = ECHILD)
                return;
            ERR("waitpid:");
        }
    }
}

void child_work(int m, int n)
{
    int count = 0;
    while(1)
    {
        for(int i = 0; i < n; i++)
        {
            const struct timespec t = {m, 0};
            nanosleep(&t, NULL);
            if(kill(getppid(), SIGUSR1))
            {
                ERR("kill:");
            }
        }
        if(kill(getppid(), SIGUSR2))
        {
            ERR("kill:");
        }
        printf("CHILD: %d\n", ++count);
    }    
}

void parent_work(sigset_t oldmask)
{
    int count = 0;
    while(1)
    {
        last_signal = 0;
        while(last_signal != SIGUSR2)
        {
            sigsuspend(&oldmask);
        }
        count++;
        printf("PARENT %d\n", count);
    }
}

int main(int argc, char** argv)
{
    int m, n;
    if (argc != 3)
    {
        usage(argv[0]);
    }
    m = atoi(argv[1]);
    n = atoi(argv[2]);
    if(m <= 0 || m > 999 || n <= 0 || n > 999)
    {
        usage(argv[0]);
    }
    set_handler(sigchld_handler, SIGCHLD);
    set_handler(sig_handler, SIGUSR1);
    set_handler(sig_handler, SIGUSR2);

    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);
    
    pid_t s;
    if((s = fork()) < 0)
    {
        ERR("fork:");
    }
    if (s == 0)
    {
        child_work(m, n);
    }
    if (s > 0)
    {
        parent_work(oldmask);
        while(wait(NULL) > 0)
        {
            ;
        }
    }
    
    return(EXIT_SUCCESS);
}