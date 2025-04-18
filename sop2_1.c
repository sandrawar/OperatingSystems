#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#define ERR(source) \
(fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

void usage(char* name)
{
    fprintf(stderr, "usage: %s 0<n\n", name);
    exit(EXIT_FAILURE);
}

void child_work(int n)
{
    srand(time(NULL) * getpid());
    const struct timespec t = {5 + (rand())%(10 - 5 + 1), 0};
    nanosleep(&t, NULL);
    printf("PROCESS with pid %d terminates\n", getpid());
}

void create_children(int n)
{
    pid_t s;
    for(n--; n>=0; n--)
    {
        if ((s = fork())<0)
        {
            ERR("Fork:");
        }
        if(!s)
        {
            child_work(n);
            exit(EXIT_SUCCESS);
        }
    }
}

int main(int argc, char** argv)
{
    int n;
    if(argc != 2)
    {
        usage(argv[0]);
    }
    n = atoi(argv[1]);
    if(n <= 0)
    {
        usage(argv[0]);
    }
    create_children(n);
    while(n > 0)
    {
        const struct timespec tk = {3, 0};
        nanosleep(&tk, NULL); 
        for(;;)
        {
            pid_t pid = waitpid(0, NULL, WNOHANG);
            if (pid > 0)
            {
                n--;
            }
            if(pid == 0)
            {
                break;
            }
            if( 0 >= pid)
            {
                if(ECHILD == errno)
                {
                    break;
                }
                ERR("waitpid:");
            }
        }
        printf("PARENT: %d proccesses ramain\n", n);
    }
}

