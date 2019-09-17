#include <stdio.h>
#include <sys/time.h>

int main(int argc, char **argv)
{
    struct timeval t;

    if (gettimeofday(&t, NULL) == 0)
        printf("%d.%06u\n", t.tv_sec, t.tv_usec);

    return 0;
}
