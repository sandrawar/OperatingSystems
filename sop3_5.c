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
#include <time.h>


#define MAXLINE 4096
#define DEFAULT_STUDENR_COUNT 10
#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

typedef unsigned int UINT;
typedef struct timespec timespec_t;
typedef struct args
{
    int* L;
    pthread_mutex_t* mxL;
    pthread_t tid;
    UINT seed;
    int* checked;
    pthread_mutex_t* mxchecked;
} args_t;

void* watekLife(void* voidArgs)
{
    args_t* args = voidArgs;
    int M = rand_r(&args->seed)%99+2;
    printf("M: %d\n",M);
    int l = 0;
    while(true)
    {
        if(l < *args->L)
        {
            l = *args->L;
            if((*(args->L)) % M == 0)
            {
                printf("%d dzieli %d\n", M, *args->L);
            }
            pthread_mutex_lock(args->mxchecked);
            *args->checked += 1;
            //printf("%d\n", *args->checked);
            pthread_mutex_unlock(args->mxchecked);
        }
    }
    return 0;
}

int main(int argc, char** argv)
{
    int w = atoi(argv[1]);
    int L = 1;
    int checked = 0;
    pthread_mutex_t mxchecked = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mxL = PTHREAD_MUTEX_INITIALIZER;
    args_t* args = (args_t*)malloc(sizeof(args_t)*w);
    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t)*w);
    srand(time(NULL));
    for(int i = 0; i < w; i++)
    {
        args[i].L = &L;
        args[i].seed = (UINT)rand();
        args[i].checked = &checked;
        args[i].mxchecked = &mxchecked;
        args[i].mxL = &mxL;
        if(pthread_create(&tid[i], NULL, watekLife, &args[i]))
            ERR("pthread create");
    }

    timespec_t s = {0};
    s.tv_sec = 0;
    s.tv_nsec = 100;
    while(true)
    {
        nanosleep(&s, &s);
        if(checked >= w)
        {
        pthread_mutex_lock(&mxchecked);
        checked = 0;
        pthread_mutex_unlock(&mxchecked);
        pthread_mutex_lock(&mxL);
        L += 1;
        pthread_mutex_unlock(&mxL);
        }
        //printf("L: %d", L);
    }
    for(int i = 0; i < w; i++)
    {
        int err = pthread_join(tid[i], NULL);
        if(err != 0)
        {
            ERR("join");
        }   
    }
    free(tid);
    return 0;
}