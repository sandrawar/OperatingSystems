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

#define ERR(source) \
    (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

#define LOTTO_SIZE 6

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s N T\n", name);
    fprintf(stderr, "N: N >= 1 - number of players\n");
    fprintf(stderr, "T: T >= 1 - number of weeks (iterations)\n");
    exit(EXIT_FAILURE);
}


void child_work(int fd, int fd2)
{
    int buffer[sizeof(int)*LOTTO_SIZE + sizeof(pid_t)];
    int *buf;
    *((pid_t *)buffer) = getpid();
    buf = buffer + sizeof(pid_t);

    /*int buffer[(int)sizeof(int)*LOTTO_SIZE];
    char *buf = (char*)malloc(sizeof(int)*LOTTO_SIZE + sizeof(pid_t));
    *((pid_t *)buf) = getpid();
    //int buf[LOTTO_SIZE + get_size(pid_t)];
    //draw(buffer);
    for(int i = 0; i < (int)sizeof(pid_t); i++)
    {
            //int r = 1 + rand() % (49 -1 + 1);
            buf[i] = (int)buf[i] - '0';
    }*/
    //TODO FOR!!!!!!!!!!!!!!!!
    for(int i = (int)sizeof(pid_t); i < (int)sizeof(int)*LOTTO_SIZE + (int)sizeof(pid_t); i++)
    {
        buf[i] = 1 + rand() % (49 -1 + 1);
    }
    //buf[(int)sizeof(pid_t)] = (char*)buffer;
    write(fd2, buf,  sizeof(int)*LOTTO_SIZE + sizeof(pid_t));
    //free(buf);
    printf("child: PROCESS with pid %d, %d, %d \n", getpid(), fd, fd2);
    read(fd, buffer, (int)sizeof(int)*LOTTO_SIZE);
    close(fd);
    close(fd2);
}

void create_children_and_pipes(int n, int *fds, int *fds2)
{
    int tmpfd[2];
    int tmpfd2[2];
    int max = n;
    while (n)
    {
        if (pipe(tmpfd) || pipe(tmpfd2))
            ERR("pipe");
        switch (fork())
        {
            case 0:
                while (n < max)
                {
                    if (fds[n] && close(fds[n]))
                        ERR("close");
                    
                    if (fds2[n] && close(fds2[n]))
                        ERR("close");
                    n+=1;
                }
                free(fds);
                free(fds2);
                if (close(tmpfd[1]))
                    ERR("close");           
                if (close(tmpfd2[0]))
                    ERR("close");
                child_work(tmpfd[0], tmpfd2[1]);
                /*if (close(tmpfd[0]))
                    ERR("close");
                if (close(tmpfd2[1]))
                    ERR("close");*/
                exit(EXIT_SUCCESS);

            case -1:
                ERR("Fork:");
        }
        if (close(tmpfd[0]))
            ERR("close");
        
        if (close(tmpfd2[1]))
            ERR("close");
        n-=1;
        fds[n] = tmpfd[1];
        fds2[n] = tmpfd2[0];
    }

    //free(fds);
    //free(fds2);
}

void parent_work(int n, int* fds, int* fds2)
{
    int buffer_p[LOTTO_SIZE * sizeof(int)];
    //int *buf = (int*)malloc(LOTTO_SIZE + sizeof(pid_t));
    //*((pid_t *)buffer) = getpid();
    //int buf[LOTTO_SIZE + get_size(pid_t)];
    draw(buffer_p);
    char* buffer_c = (char*)buffer_p;
    //buff_c = buffer + sizeof(pid_t);
    for(int i = 0; i < n; i++)
    {
        //printf("parent\n");
        write(fds[i], buffer_c,  sizeof(int) * LOTTO_SIZE);
    }
    for(int i = 0; i < n; i++)
    {
        int buf[LOTTO_SIZE * sizeof(int) + sizeof(pid_t)];
        read(fds2[i], buf, sizeof(int) * LOTTO_SIZE + sizeof(pid_t));
        printf("from: ");
        for(int j = 0; j < 4; j++)
        {
            printf("%d", buf[j]);
        }
        printf(" gets: ");
        for(int j = 4; j <  4 + LOTTO_SIZE; j++)
        {
            printf("%d ", buf[j]);
        }
        printf("\n");
    }
}

int main(int argc, char **argv)
{
    pid_t pid = getpid();
    srand(pid);
    if(argc != 3)
        usage(argv[0]);
    int n = atoi(argv[1]);
    int* fds;
    int* fds2;
    if (NULL == (fds = (int *)malloc(sizeof(int) * n)))
        ERR("malloc");
    if (NULL == (fds2 = (int *)malloc(sizeof(int) * n)))
        ERR("malloc");
    
    create_children_and_pipes(n, fds, fds2);
    parent_work(n, fds, fds2);
    
    /*int bet[n], numbers[n];

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

    //usage(argv[0]);*/
    //free(fds);
    //free(fds2);
    exit(EXIT_SUCCESS);
}
