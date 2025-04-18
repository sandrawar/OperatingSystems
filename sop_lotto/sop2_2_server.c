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
    mqd_t *s;
    char ni[MAX_MSG_SIZE];
    unsigned msg_prio;

    s = (mqd_t *)info->si_value.sival_ptr;

    static struct sigevent not ;
    not .sigev_notify = SIGEV_SIGNAL;
    not .sigev_signo = SIGRTMIN;
    not .sigev_value.sival_ptr = s;
    
    if (mq_notify(*s, &not ) < 0)
        ERR("mq_notify");

    for (;;)
    {
        //printf("message\n");
        if (mq_receive(*s, ni, MAX_MSG_SIZE, &msg_prio) < 1)
        {
            if (errno == EAGAIN)
                break;
            else
                ERR("mq_receive");
        }
        printf("message: %s\n", ni);
        mqd_t p_q;
        char *token = strtok((char *)&ni, " ");
        if (token == NULL)
        {
            fprintf(stderr, "Invalid message received\n");
            break;
        }
        printf("%s\n", token);
        int pid_q_i = atoi(token);
        char pid_q[sizeof(int)+5*sizeof(char)];
        snprintf(pid_q, sizeof(pid_q), "/%d", pid_q_i); 
    
        struct mq_attr attr;
        attr.mq_maxmsg = 10;
        attr.mq_msgsize = 100;
    
        if ((p_q = mq_open(pid_q, O_WRONLY | O_CREAT, 0600, &attr)) == (mqd_t)-1)
            ERR("mq open");
        token = strtok(NULL, " ");
        int t2 = atoi(token);
        printf("%d\n", t2);
        token = strtok(NULL, " ");

        int t3 = atoi(token);
        printf("%d\n", t3);
        uint16_t n = (uint16_t)t2 + (uint16_t)t3;
        printf("%d\n", n);
        if (mq_send(p_q, (const char *)&n, sizeof(uint16_t), 0))
            ERR("mq_send");
        printf("sent\n");
    }
}

int main(int argc, char* argv[])
{
    mqd_t s, d, m;
    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 100;
    pid_t pid = getpid();
    
    char name_s[sizeof(pid)+5*sizeof(char)]; 
    snprintf(name_s, sizeof(name_s), "/%d_s", pid);
    
    mq_unlink(name_s); 
    if ((s = mq_open(name_s, O_RDONLY | O_CREAT, 0600, &attr)) == (mqd_t)-1)
        ERR("mq open");
    printf("%s\n", name_s);

    char name_d[sizeof(pid)+5*sizeof(char)]; 
    snprintf(name_d, sizeof(name_d), "/%d_d", pid); 
    if ((d = mq_open(name_d, O_WRONLY | O_CREAT, 0600, &attr)) == (mqd_t)-1)
        ERR("mq open");
    printf("%s\n", name_d);

    char name_m[sizeof(pid)+5*sizeof(char)]; 
    snprintf(name_m, sizeof(name_m), "/%d_m", pid); 
    if ((m = mq_open(name_m, O_WRONLY | O_CREAT, 0600, &attr)) == (mqd_t)-1)
        ERR("mq open");
    printf("%s\n", name_m);
    
    sethandler(mq_handler, SIGRTMIN);
    
    static struct sigevent noti;  
    noti.sigev_notify = SIGEV_SIGNAL;
    noti.sigev_signo = SIGRTMIN;
    noti.sigev_value.sival_ptr = &s;
    if (mq_notify(s, &noti) < 0)
        ERR("mq_notify");
    //sleep(1);
    
    while(1) {
        pause(); // Czekaj na sygnał
    }
    
    mq_close(s);
    mq_close(d);
    mq_close(m);
    return 0;
}
