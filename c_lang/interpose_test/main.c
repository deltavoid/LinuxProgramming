#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

// extern __pid_t getpid (void) __THROW;

int main()
{
    int pid = getpid();
    printf("pid1: %d\n", pid);

    sleep(1);
    pid = getpid();
    printf("pid2: %d\n", pid);

    return 0;
}