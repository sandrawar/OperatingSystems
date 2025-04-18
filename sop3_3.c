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
#define DEFAULT_ARRAYSIZE 10
#define DELETED_ITEM -1
#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

typedef struct argsSignalHandlers
{
    pthread_t tid;
    sigset_t* pMask;
    int* array;
    int* pArrayCount;
    pthread_mutex_t* pmxArray;
    bool* pQuitFlag;
    pthread_mutex_t* pmxQuitFlag;
} argsSignalHandlers_t;

void ReadArguments(int argc, char** argv, int* arraySize);
void removeItem(int* array, int* arrayCount, int index);
void printArray(int* array, int arraySize);
void*signal_handling(void*);

int main(int argc, char** argv)
{
    int arraySize;
    int *array;
    bool quitFlag = false;
    pthread_mutex_t pmxQuitFlag = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t pmxArray = PTHREAD_MUTEX_INITIALIZER;
    ReadArguments(argc, argv, &arraySize);
    printf("%d\n", arraySize);
    if(NULL == (array = (int*)malloc(sizeof(int)*arraySize)))
        ERR("malloc");
    for(int i = 0; i < arraySize; i++)
    {
        array[i] = i + 1;
    }
    sigset_t oldMask, newMask;
    sigemptyset(&newMask);
    sigaddset(&newMask, SIGINT);
    sigaddset(&newMask, SIGQUIT);
    if(pthread_sigmask(SIG_BLOCK, &newMask, &oldMask))
        ERR("mask");

    argsSignalHandlers_t args;
    args.pMask = &newMask;
    args.array = array;
    args.pArrayCount = &arraySize;
    args.pQuitFlag = &quitFlag;
    args.pmxArray = &pmxArray;
    args.pmxQuitFlag = &pmxQuitFlag;
    if(pthread_create(&args.tid, NULL, signal_handling, &args))
        ERR("create thread");
    while(true)
    {
        pthread_mutex_lock(&pmxQuitFlag);
        if(quitFlag)
        {
            pthread_mutex_unlock(&pmxQuitFlag);
            break;
        }
        pthread_mutex_unlock(&pmxQuitFlag);

        pthread_mutex_lock(&pmxArray);
        printArray(array, *args.pArrayCount);
        pthread_mutex_unlock(&pmxArray);
        sleep(1);
    }
    free(array);


}

void ReadArguments(int argc, char** argv, int* arraySize)
{
    *arraySize = DEFAULT_ARRAYSIZE;
    if (argc >= 2)
    {
        *arraySize = atoi(argv[1]);
        if(*arraySize <= 0)
        {
            printf("Invalid arguments\n");
            exit(EXIT_FAILURE);
        }
    }
}

void removeItem(int* array, int* arrayCount, int index)
{
    for(int i = index; i < *arrayCount-1; i++)
    {
        array[i] = array[i+1];
    }
    *arrayCount--;
    return;
}

void printArray(int* array, int arraySize)
{
    printf("Array: [");
    for(int i = 0; i < arraySize; i++)
    {
        printf("%d, ", array[i]);
    }
    printf("]\n");
    return;
}

void* signal_handling(void* voidArgs)
{
    argsSignalHandlers_t* args = voidArgs;
    int signo;
    srand(time(NULL));
    for(;;)
    {
    if(sigwait(args->pMask, &signo))
        ERR("sigwait");
    switch(signo)
    {
        case SIGINT:
            pthread_mutex_lock(args->pmxArray);
            if(*args->pArrayCount > 0)
                removeItem(args->array, args->pArrayCount,rand() % (*args->pArrayCount));
                *args->pArrayCount--;
            pthread_mutex_unlock(args->pmxArray);
            break;
        case SIGQUIT:
            pthread_mutex_lock(args->pmxQuitFlag);
            *args->pQuitFlag = true;
            pthread_mutex_unlock(args->pmxQuitFlag);
            return NULL;
        default:
            exit(1);
    }
    }
    return NULL; 
}