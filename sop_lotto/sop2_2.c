#define _GNU_SOURCE
#include <errno.h>
#include <mqueue.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <signal.h> // You need to include signal.h for SIGKILL

#define LIFE_SPAN 10
#define MAX_NUM 10
#define MAX_MSG_SIZE 100

#define ERR(source, ...) \
    do { \
        fprintf(stderr, "%s:%d ", __FILE__, __LINE__); \
        perror(source); \
        kill(0, SIGKILL); \
        exit(EXIT_FAILURE); \
    } while(0)

void sethandler(void (*f)(int, siginfo_t *, void *), int sigNo)
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_sigaction = f;
    act.sa_flags = SA_SIGINFO;
    if (-1 == sigaction(sigNo, &act, NULL))
        ERR("sigaction");
}

void mq_handler(int sig, siginfo_t *info, void *p)
{
    mqd_t *client;
    uint16_t ni;
    unsigned msg_prio;

    client = (mqd_t *)info->si_value.sival_ptr;

    static struct sigevent not ;
    not .sigev_notify = SIGEV_SIGNAL;
    not .sigev_signo = SIGRTMIN;
    not .sigev_value.sival_ptr = client;
    
    if (mq_notify(*client, &not ) < 0)
        ERR("mq_notify");

    for (;;)
    {
        //printf("message\n");
        if (mq_receive(*client, (char *)&ni, 100*sizeof(uint16_t), &msg_prio) < 1)
        {
            if (errno == EAGAIN)
                break;
            else
                ERR("mq_receive");
        }
        else{
            printf("message: %d\n", ni);
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc != 2)
        ERR("arguments");

    mqd_t client, server_s;
    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 100;
    pid_t pid = getpid();
    char name[sizeof(pid)+5*sizeof(char)]; 
    snprintf(name, sizeof(name), "/%d", pid); 
    
    if ((client = mq_open(name, O_RDONLY | O_CREAT, 0600, &attr)) == (mqd_t)-1)
        ERR("mq open");
    if ((server_s = mq_open(argv[1], O_WRONLY | O_CREAT, 0600, &attr)) == (mqd_t)-1)
        ERR("mq open");
    printf("%s\n", name);

    int liczba1, liczba2;
    scanf("%d", &liczba1);
    scanf("%d", &liczba2);


    char line[MAX_MSG_SIZE]; 
    snprintf(line, sizeof(line), "%d %d %d ", pid, liczba1, liczba2);
    if (mq_send(server_s, line, MAX_MSG_SIZE, 0))
            ERR("mq_send"); 

    sethandler(mq_handler, SIGRTMIN);
    static struct sigevent noti;  
    noti.sigev_notify = SIGEV_SIGNAL;
    noti.sigev_signo = SIGRTMIN;
    noti.sigev_value.sival_ptr = &client;
    if (mq_notify(client, &noti) < 0)
        ERR("mq_notify");

    while(1) {
        pause(); // Czekaj na sygnał
    }
    
    mq_close(client);
    mq_close(server_s);
    return 0;
}
