#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void usage(char *pname)
{
    fprintf(stderr, "ESAGE: %s name times>0", pname);
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv)
{
    if(argc != 3)
    {
        usage(argv[0]);
    }

    int times = atoi(argv[2]);
    
    if(times == 0)
    {
        usage(argv[0]);
    }
    
    for(int i = 0; i < times; i++)
    {
        printf("Hello, %s\n", argv[1]);
    }
    return EXIT_SUCCESS;
}