#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ERR(source) (perror(source),\
fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
exit(EXIT_FAILURE))

int main(int argc, char** argv)
{
    char name[22];
    while(fgets(name, 22, stdin) != NULL)
        printf("Hello, %s", name);
    return EXIT_SUCCESS;
}