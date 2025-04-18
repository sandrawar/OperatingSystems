#define _GNU_SOURCE
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

#define MAX_PATH 100
#define MAX_LEN 1000000
#define MAXFD 20

ssize_t bulk_read(int fd, char *buf, size_t count)
{
    ssize_t c;
    ssize_t len = 0;
    do
    {
        c = TEMP_FAILURE_RETRY(read(fd, buf, count));
        if (c < 0)
            return c;
        if (c == 0)
            return len;  // EOF
        buf += c;
        len += c;
        count -= c;
    } while (count > 0);
    return len;
}

ssize_t bulk_write(int fd, char *buf, size_t count)
{
    ssize_t c;
    ssize_t len = 0;
    do
    {
        c = TEMP_FAILURE_RETRY(write(fd, buf, count));
        if (c < 0)
            return c;
        buf += c;
        len += c;
        count -= c;
    } while (count > 0);
    return len;
}

 void show_stage2(const char *const path, const struct stat *const stat_buf) 
 {
    printf("stage2\n"); 
    if(S_ISREG(stat_buf->st_mode))
    {
        printf("size: %ld\n", stat_buf->st_size);
        printf("zawartosć:\n");
        const int fd = open(path, O_RDONLY);
        if (fd == -1)
            ERR("open");
        char file_buf[MAX_PATH + 1];
        for (;;)
        {
            const ssize_t read_size = bulk_read(fd, file_buf, MAX_PATH);
            file_buf[read_size] = '\0';
            if (read_size == -1)
                ERR("bulk_read");

            if (read_size == 0)
                break;
            printf("%s\n", file_buf);
        }
        if (close(fd))
            ERR("fclose");
    } else if(S_ISDIR(stat_buf->st_mode))
    {
        DIR* dirp;
        struct dirent* dp;
        if (NULL == (dirp = opendir(path)))
            ERR("opendir");
        do
        {
            errno = 0;
            if ((dp = readdir(dirp)) != NULL)
            {   
                printf("%s\n", dp->d_name);
            }
    } while (dp != NULL);
        closedir(dirp);
    }

return;}

 void write_stage3(const char *const path, const struct stat *const stat_buf) 
 {
    printf("stage3\n"); 
    show_stage2(path, stat_buf);
    const int fd = open(path, O_APPEND | O_WRONLY);
    char line[MAX_PATH];
    do
    {
        fgets(line, MAX_PATH, stdin);
        if(strcmp(line, "\n") == 0)
        {
            break;
        }
        line[strlen(line)] = '\0';
        write(fd, line, strlen(line));
    } while (1);
    close(fd);
    return;
}

int nfFun(const char* name, const struct stat* filestat, int type, struct FTW* f)
{
    (void)type;
    (void)f;
    switch(type)
    {
        case FTW_D:
            printf("katalog: %s\n", name);
            break;
        case FTW_F:
            printf("plik: %s\n", name);
            break;
        default:
            printf("nieznany: %s\n", name);
            break;
    }
    return 0;
}

 void walk_stage4(const char *const path, const struct stat *const stat_buf) 
 {
    (void)stat_buf;
    printf("stage4\n"); 
    nftw(path, nfFun, MAXFD, FTW_PHYS);
    return;
}

int interface_stage1() {
    printf("1. show\n");
    printf("2. write\n");
    printf("3. walk\n");
    printf("4. exit\n");
    char N[MAX_PATH];
    fgets(N, MAX_PATH, stdin);
    N[strlen(N) - 1] = '\0';
    int n = N[0] - '0';
    if(!(n == 1 || n == 2 || n == 3 || n == 4))
    {
        printf("Numer polecenia: %d\n", n);
        printf("Błędny numer polecenia\n");
        return 1;
    }
    char path[MAX_PATH];
    struct stat st;
    fflush(stdin);
    switch(n)
    {
        case 1:
            printf("podaj ścieżkę:\n");
            fgets(path, MAX_PATH, stdin);
            path[strlen(path) - 1] = '\0';
            if(stat(path, &st)){printf("incorrect path\n"); return 1;}
            show_stage2(path, &st);
            break;
        case 2:
            printf("podaj ścieżkę:\n");
            fgets(path, MAX_PATH, stdin);
            path[strlen(path) - 1] = '\0';
            if(stat(path, &st)){printf("incorrect path\n"); return 1;}
            write_stage3(path, &st);
            break;
        case 3:
            printf("podaj ścieżkę:\n");
            fgets(path, MAX_PATH, stdin);
            path[strlen(path) - 1] = '\0';
            if(stat(path, &st)){printf("incorrect path\n"); return 1;}
            walk_stage4(path, &st);
            break;
        case 4:
            return 0;
        default:
            ERR("stage1");
    }

    return 1;
}

int main()
{
    while (interface_stage1())
        ;
    return EXIT_SUCCESS;
}
