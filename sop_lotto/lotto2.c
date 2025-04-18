#define _GNU_SOURCE
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/wait.h>

#include "utils.h"
#define LOTTO_COUNT 6
//#define ERR(source)(fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), exit(EXIT_FAILURE))
#define ERR(source) \
    (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

typedef struct pipe_t
{
    int fd[2];
}pipe_t;
void usage(char *name)
{
    fprintf(stderr, "USAGE: %s N T\n", name);
    fprintf(stderr, "N: N >= 1 - number of players\n");
    fprintf(stderr, "T: T >= 1 - number of weeks (iterations)\n");
    exit(EXIT_FAILURE);
}
int sethandler(void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
    if (-1 == sigaction(sigNo, &act, NULL))
        return -1;
    return 0;
}

void child_work(int id,pipe_t pipefdsRid,pipe_t pipefdsWid)
{
    srand(time(NULL) *getpid());

    while(1)
    {
        int shouldILose = rand() % 99;
        if(shouldILose >= 80)
        {
            
             if(close(pipefdsRid.fd[1]))
                ERR("close");
             if(close(pipefdsWid.fd[0]))
                ERR("close");
            printf("%d i quit %d \n",id,shouldILose);
            exit(EXIT_SUCCESS);
        }
        //printf("[%d]: I m going to play lottto\n",id);
        int lotto[LOTTO_COUNT];
        for(int i=0;i<LOTTO_COUNT;i++)
        {
            int r = 1 + rand() % (49 -1 + 1);
            lotto[i] = r;
            //printf("%d send %d\n", id,lotto[i]);
        }
        if(write(pipefdsRid.fd[1],lotto,sizeof(int)*LOTTO_COUNT)<0)
            ERR("write");
        int winners[LOTTO_COUNT];
        if(read(pipefdsWid.fd[0],winners,sizeof(int)*LOTTO_COUNT) == 0)
        {
            if(close(pipefdsRid.fd[1]))
                ERR("close");
             if(close(pipefdsWid.fd[0]))
                ERR("close");
            exit(EXIT_SUCCESS);
        }
        int matches = compare(lotto,winners);
        int reward = get_reward(matches);
        printf("%d winner %d\n",id,reward);
    }
    

}
void parentStartClosePipe(pipe_t* pipefdsR,pipe_t* pipefdsW,int n)
{
    for(int i=0;i<n;i++)
    {
        if(close(pipefdsR[i].fd[1]))
            ERR("Close");
        if(close(pipefdsW[i].fd[0]))
            ERR("close");
    }
}
void childClosePipe(pipe_t* pipefdsR,pipe_t* pipefdsW,int n, int id)
{
    for(int i=0;i<n;i++)
    {
        if(i!=id)
        {
            if(close(pipefdsR[i].fd[1]))
                ERR("close");
            if(close(pipefdsR[i].fd[0]))
                ERR("close");
            if(close(pipefdsW[i].fd[1]))
                ERR("close");
            if(close(pipefdsW[i].fd[0]))
                ERR("close");
        }
        else 
        {
            if(close(pipefdsR[i].fd[0]))
                ERR("close");
            if(close(pipefdsW[i].fd[1]))
                ERR("close");
        }

    }
}
void create_process(int n,pipe_t* pipefdsR,pipe_t* pipefdsW)
{
    int N = n;
    while(n-->0)
    {
        switch(fork())
        {
            case -1:
                ERR("fork");
            case 0:
                childClosePipe(pipefdsR,pipefdsW,N,n);
                child_work(n,pipefdsR[n],pipefdsW[n]);

                exit(EXIT_SUCCESS);
            default:
                break;

        }
    }
}

int main(int argc, char **argv)
{
    
    pid_t pid = getpid();
    srand(pid);
    

    //int bet[6], numbers[6];
    /*
    for (int i = 0; i < 100; ++i) {
        draw(bet);
        draw(numbers);
        int matches = compare(bet, numbers);
        int reward = get_reward(matches);
        if(reward == 0) continue;
        printf("bet: %d, %d, %d, %d, %d, %d\n", bet[0], bet[1], bet[2], bet[3], bet[4], bet[5]);
        printf("draw: %d, %d, %d, %d, %d, %d\n", numbers[0], numbers[1], numbers[2], numbers[3], numbers[4], numbers[5]);
        printf("%d numbers matches\n", matches);
        printf("%d payout\n", reward);
    }
    */
    
    if(argc != 3)
        usage(argv[0]);
        
    int n = atoi(argv[1]);
    int t = atoi(argv[2]);
    pipe_t* pipefdsR = (pipe_t*)malloc(sizeof(pipe_t)*n);
    if(!pipefdsR)
        ERR("malloc");
    pipe_t* pipefdsW = (pipe_t*)malloc(sizeof(pipe_t)*n);
    if(!pipefdsW)
        ERR("malloc");
    for(int i= 0;i<n;i++)
    {
        int fd[2];
        if(pipe(fd))
            ERR("pipe");
        pipefdsR[i].fd[0] = fd[0];
        pipefdsR[i].fd[1] = fd[1];

        if(pipe(fd))
            ERR("pipe");
        pipefdsW[i].fd[0] = fd[0];
        pipefdsW[i].fd[1] = fd[1];

    }
    create_process(n,pipefdsR,pipefdsW);
    parentStartClosePipe(pipefdsR, pipefdsW,n);
    sethandler(SIG_IGN,SIGPIPE);
    
    int bets = 0;
    int rewardCount = 0;
    int amountWon = 0;
    int T=0;
    int winners[LOTTO_COUNT];

    while(T++ < t)
    {
        draw(winners);
        for(int i=0;i<n;i++)
        {
            printf("%d week\n",T);
            int numbers[LOTTO_COUNT];
            if(read(pipefdsR[i].fd[0],numbers,sizeof(int)*LOTTO_COUNT)<0)
                ERR("read");
            printf("from %d get \n",i);
            bets += 3;
            for(int j=0;j<LOTTO_COUNT;j++)
            {
                printf(" %d",numbers[j]);
            }
            int matches = compare(numbers,winners);
            if( matches > 0)
            {
                amountWon += get_reward(matches);
                rewardCount += 1;
            }
            puts("\n");
        }
    
        printf("winners\n");
        for(int i=0;i<LOTTO_COUNT;i++)
        {
            printf("%d ",winners[i]);
        }
        printf("\n");
        
        

        for(int i=0;i<n;i++)
        {
            if(write(pipefdsW[i].fd[1],winners,sizeof(int)*LOTTO_COUNT)<0)
                if(errno != EPIPE)
                    ERR("write");
        }
    }
    puts("test\n");
    for(int i=0;i<n;i++)
    {
        if(close(pipefdsR[i].fd[0]))
            ERR("close");
        if(close(pipefdsW[i].fd[1]))
            ERR("close");
    }
    while(n-->0)
    {
        while(wait(NULL)> 0) {}
    }
    printf("%d bets %d rewardCount %d amountWon\n",bets,rewardCount,amountWon);

    free(pipefdsR);
    free(pipefdsW);
    
    exit(EXIT_SUCCESS);
    
}