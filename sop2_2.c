#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#define ERR(source) \
(fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

volatile sig_atomic_t last_signal = 0;

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

void sig_handler(int sig#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/time.h>)
{
    printf("[%d] received signal %d\n", getpid(), sig);
    last_signal = sig;
}

void sigchld_handler(int sig)
{
    pid_t pid;
    for(;;)
    {
        pid = waitpid(0, NULL, WNOHANG);
        if (pid == 0)
        {
            return;
        }
        if (pid <= 0)
        {
            if (errno == ECHILD)
            {
                return;
            }
            ERR("waitpid:");
        }
    }
}

void usage(char* name)
{
    fprintf(stderr, "usage: %s n>0 k>0 p>0 l>0\n", name);
    exit(EXIT_FAILURE);
}

void child_work(int l)
{
    int t, tt;
    srand(getpid());
    t = 5 + rand()%6;
    while(l-- > 0)
    {
        for(tt = t; tt > 0; tt = sleep(tt))
        {
            ;
        }
        if(last_signal == SIGUSR1)
        {
            printf("SUCCESS [%d]\n", getpid());
        } else
        {
            printf("FAILURE, [%d]\n", getpid());
        }
    }
    printf("[%d] Terminates \n", getpid());
}

void create_children(int n, int l)
{
    for(n--; n > 0; n--)
    {
        int s;
        if((s = fork()) < 0)
        {
            ERR("Fork:");
            exit(EXIT_FAILURE);
        }
        if(!s)
        {
            set_handler(sig_handler, SIGUSR1);
            set_handler(sig_handler, SIGUSR2);
            child_work(l);
            exit(EXIT_SUCCESS);
        }
    }
}

void parent_work(int k, int p, int l)
{
    struct timespec tk = {k, 0};
    struct timespec tp = {p, 0};
    set_handler(sig_handler, SIGALRM);
    alarm(l * 10);
    while(last_signal != SIGALRM)
    {
        nanosleep(&tk, NULL);
        if(kill(0, SIGUSR1) < 0)
        {
            ERR("kill:");
        }
        nanosleep(&tp, NULL);
        if(kill(0, SIGUSR2) < 0)
        {
            ERR("kill:");
        }
    }
    printf("[PARENT] Terminates \n");
}

int main(int argc, char** argv)
{
    if (argc != 5)
    {
        usage(argv[0]);
    }
    int n, k, p, l;
    n = atoi(argv[1]);
    k = atoi(argv[2]);
    p = atoi(argv[3]);
    l= atoi(argv[4]);

    set_handler(sigchld_handler, SIGCHLD);
    set_handler(SIG_IGN, SIGUSR1);
    set_handler(SIG_IGN, SIGUSR2);
    create_children(n, l);
    parent_work(k, p, l);
    while(wait(NULL) > 0)
    {
        ;
    }
    return EXIT_SUCCESS;
}