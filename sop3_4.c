#include <unistd.h>
#include <pthread.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>


#define MAXLINE 4096
#define DEFAULT_STUDENR_COUNT 100
#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

typedef unsigned int UINT;
typedef struct timespec timespec_t;
typedef struct yearCounters
{
    int* Years;
    pthread_mutex_t* mxCounters;
} yearCounters_t;


void* StudentWork(void* voidArgs)
{
    yearCounters_t* args = voidArgs;
    int i = 0;
    for(int j = 0; j < 3; j++)
    {
        sleep(1);
        pthread_mutex_lock(args->mxCounters);
        args->Years[i]--;
        args->Years[++i]++;
        pthread_mutex_unlock(args->mxCounters);
    }
    return NULL;
}

int main(int argc, char** argv)
{
    int n = DEFAULT_STUDENR_COUNT;
    if(argc >= 2)
    {
        n = atoi(argv[1]);
    }
    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t)*n);
    int* array = (int*)malloc(sizeof(int)*4);
    if(NULL == array || NULL == tid)
    {
        ERR("malloc");
    }
    array[0] = n;
    for(int i = 1; i < 4; i++)
    {
        array[i] = 0;
    }
    pthread_mutex_t Mutex = PTHREAD_MUTEX_INITIALIZER;
    yearCounters_t yearCounters;
    yearCounters.Years = array;
    yearCounters.mxCounters = &Mutex;
    srand(time(NULL));
    for(int i = 0; i < n; i++)
    {
        if(pthread_create(&tid[i], NULL, StudentWork, &yearCounters))
            ERR("pthread create");
    }
    timespec_t s = {0};
    for(int i = 0; i < 4; i++)
    {
        s.tv_sec = rand() % 4;
        s.tv_nsec = rand() % 201 + 100;
        nanosleep(&s, &s);
        int kicked = rand()%n;
        n--;
        pthread_cancel(tid[kicked]);
    }
    pthread_mutex_destroy(&Mutex);
    //free(array);
    //free(yearCounters);
    printf("%d %d %d %d\n", yearCounters.Years[0], yearCounters.Years[1], yearCounters.Years[2], yearCounters.Years[3]);
    exit(EXIT_SUCCESS);
}