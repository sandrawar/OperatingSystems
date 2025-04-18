#define GNU_SOURCE
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

struct arguments
{
    int id;
    int hand[7];
    pthread_barrier_t* barrier; 
};

void sigint_handler(int sig)
{
    return;
}

int set_handler(void(*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
    if (-1 == sigaction(sigNo, &act, NULL))
        return -1;
    return 0;
}

void* thread_func(void* voidargs)
{
    struct arguments* args = (struct arguments*)voidargs;
    fprintf(stderr, "[%d] Hand: ", args->id);
    for(int i = 0; i < 6; i++)
    {
        fprintf(stderr, "%d, ", args->hand[i]);
    }
    fprintf(stderr, "%d\n", args->hand[6]);
    return NULL;
}

void create_threads(pthread_t* players, struct arguments* args, int n)
{
    for(int i = 0; i < n; i++)
    {
        pthread_create(&players[i], 0, thread_func, (void*)&args[i]);
    }
    return;
}

int main(int argc, char** argv)
{
    if(argc != 2)
        ERR("usage");
    int n = atoi(argv[1]);
    pthread_t players[n];
    struct arguments* args = (struct arguments*)malloc(sizeof(struct arguments)*n);
    pthread_barrier_t barrier;
    sigset_t old;
    sigmask_t new;
    

    pthread_barrier_init(&barrier, NULL, n);
    if(set_handler(sigint_handler, SIGINT))
        ERR("setting SIGINT");
    
    srand(time(NULL));
    
    for(int i = 0; i < n; i++)
    {
        srand(rand());
        for(int j = 0; j < 7; j++)
        {
            args[i].hand[j] = rand() % 52;
        }
        args[i].barrier = &barrier;
        args[i].id = i + 1;
    }

    create_threads(players, args, n);

    for(int i = 0; i < n; i++)
    {
        pthread_join(players[i], NULL);
    }
    free(args);
    pthread_barrier_destroy(&barrier);
    exit(EXIT_SUCCESS);
}