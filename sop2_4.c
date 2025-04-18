#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#define ERR(source)\
(fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

volatile sig_atomic_t sig_count = 0;

void usage(char* name)
{
    fprintf(stderr, "usage: %s m p", name);
    exit(EXIT_FAILURE);
}

void set_handler(void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
    if(-1 == sigaction(sigNo, &act, NULL))
    {
        ERR("sigaction:");
    }
}

void sig_handler(int sig)
{
    sig_count++;
}

void child_work(int t)
{
    while(1)
    {
        struct timespec tt = {0, t * 10000};
        set_handler(SIG_DFL, SIGUSR1);
        nanosleep(&tt, NULL);
        if(kill(getppid(), SIGUSR1))
        {
            ERR("kill:");
        }
    }    
}

ssize_t bulk_read(int fd, char* buf, size_t count)
{
    ssize_t c;
    ssize_t len = 0;
    do
    {
        c = TEMP_FAILURE_RETRY(read(fd, buf, count));
        if (c < 0)
            return c;
        if (c == 0)
            return len;
        buf += c;
        len += c;
        count -= c;
    } while (count > 0);
    return len;
}

ssize_t bulk_write(int fd, char* buf, size_t count)
{
    ssize_t c;
    ssize_t len = 0;
    do
    {
        c = TEMP_FAILURE_RETRY(write(fd, buf, count));
        if (c < 0)
            return c;
        if (c == 0)
            return len;
        buf += c;
        len += c;
        count -= c;
    } while (count > 0);
    return len;
}

void parent_work(int b, int s, char* name)
{
    int i, in, out;
    ssize_t count;
    char* buf = malloc(s);
    if(!buf)
    {
        ERR("malloc:");
    }
    if((out =TEMP_FAILURE_RETRY(open(name, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0777))) < 0)
        ERR("open:");
    if((in = TEMP_FAILURE_RETRY(open("/dev/urandom", O_RDONLY))) < 0)
        ERR("open:");
    for (i = 0; i < b; i++)
    {
        if (count = bulk_read(in, buf, s) < 0)
            ERR("read:");
        if (count = bulk_write(out, buf, count) < 0)
            ERR("read");
        if (TEMP_FAILURE_RETRY(fprintf(stderr, "Blok of %ld bytes transfered. Signals RX:%d\n", count, sig_count)) < 0)
            ERR("fprintf:");
    }
    if(TEMP_FAILURE_RETRY(close(in)))
        ERR("close");
    if(TEMP_FAILURE_RETRY(close(out)))
        ERR("close:");
    free(buf);if(kill(0, SIGUSR1))
        ERR("kill:");
}

int main(int argc, char** argv)
{
    int t, b, s;
    if (argc != 5)
    {
        usage(argv[0]);
    }
    char* name = argv[4];
    t = atoi(argv[1]);
    b = atoi(argv[2]);
    s = atoi(argv[3]);
    if(t <= 0 || t> 999 || b <= 0 || s <= 0)
    {
        usage(argv[0]);
    }

    set_handler(sig_handler, SIGUSR1);

    pid_t sf;
    if((sf = fork()) < 0)
    {
        ERR("fork:");
    }
    if (sf == 0)
    {
        child_work(t);
    }
    if (sf > 0)
    {
        parent_work(b, s * 1024 * 1024, name);
        while(wait(NULL) > 0)
        {
            ;
        }
    }
    
    return(EXIT_SUCCESS);
}