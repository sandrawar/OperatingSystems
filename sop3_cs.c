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


#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

typedef struct args
{
    int* array;
    int* size;
    pthread_mutex_t* mxarray;
    pthread_mutex_t* mxcount;
    sigset_t* mask;
    int* pcount;
} args_t;

void* printing_func(void* argsvoid)
{
    args_t* args = argsvoid;
    printf("pw:%d\n", *args->pcount);
    printf("[");
    pthread_mutex_lock(args->mxarray);
    for(int i = 0; i < *args->size; i++)
        printf("%d ", args->array[i]);
    pthread_mutex_unlock(args->mxarray);
    printf("]\n"); 
    pthread_mutex_lock(args->mxcount);
    *args->pcount -= 1;
    pthread_mutex_unlock(args->mxcount);  
}

void* swaping_fun(void* voidargs)
{
    args_t* args = voidargs;
    printf("pw:%d\n", *args->pcount);
        int a = rand() % (*args->size);
        int b = rand() % (*args->size - a) + a;
        while(a < b)
        {
            pthread_mutex_lock(args->mxarray);
            int tmp = args->array[a];
            args->array[a] = args->array[b];
            args->array[b] = tmp;
            pthread_mutex_unlock(args->mxarray);
            a++;
            b--;
        }
    pthread_mutex_lock(args->mxcount);
    *args->pcount -= 1;
    pthread_mutex_unlock(args->mxcount);
        
}

int main(int argc, char** argv)
{
    pid_t pid = getpid();
    printf("%d\n", pid);
    if(argc != 3)
        ERR("arguments");
    int n = atoi(argv[1]);
    int p = atoi(argv[2]);
    pthread_t* tids = (pthread_t*)malloc(sizeof(pthread_t)*p);
    srand(time(NULL));
    int pw = 0;
    int* array = (int*)malloc(sizeof(int)*n);
    if(array == NULL) 
        ERR("malloc");
    for(int i = 0; i < n; i++)
    {
        array[i] = i;
    }

    sigset_t oldMask, newMask;
    sigemptyset(&newMask);
    sigaddset(&newMask, SIGUSR1);
    sigaddset(&newMask, SIGUSR2);
    if(pthread_sigmask(SIG_BLOCK, &newMask, &oldMask))
        ERR("sigblock");
    
    args_t args;
    pthread_mutex_t mutexarray = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mxpw = PTHREAD_MUTEX_INITIALIZER;
    args.array = array;
    args.size = &n;
    args.mask = &newMask;
    args.pcount = &pw;
    args.mxarray = &mutexarray;
    args.mxcount = &mxpw;
    pthread_attr_t threadAttr;
    if(pthread_attr_init(&threadAttr))
        ERR("attr");
    if(pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED))
        ERR("detach");
    int signo;
    srand(time(NULL));
    for(;;)
    {
        if(sigwait(&newMask, &signo))
            ERR("sigwait");
        switch(signo)
        {
            case SIGUSR1:
                if(pw<p)
                {
                    if(pthread_create(&tids[pw-1], &threadAttr, printing_func, &args))
                        ERR("create_thread");
                    pthread_mutex_lock(&mxpw);
                    pw+=1;
                    pthread_mutex_unlock(&mxpw);
                }
                else 
                {
                    printf("too much:%d\n", pw);
                }
            case SIGUSR2:
                if(pw<p)
                {
                    if(pthread_create(&tids[pw-1], &threadAttr, swaping_fun, &args))
                        ERR("create_thread");
                    pthread_mutex_lock(&mxpw);
                    pw+=1;
                    pthread_mutex_unlock(&mxpw);
                }
                else {printf("too much:%d\n", pw);}
        }
    }
    
    free(array);
    pthread_attr_destroy(&threadAttr);
}