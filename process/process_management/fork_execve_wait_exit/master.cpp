#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


const char worker_path[] = "worker";
const int worker_num = 4;

int main()
{
    char id_str[10];

    for (int id = 0; id < worker_num; id++)
    {   int pid = 0;
        sprintf(id_str, "%d", id);

        if  ((pid = fork()) == 0)
        {
            execl(worker_path, worker_path, id_str, NULL);

            exit(-1);
        }
        else
        {
            printf("%d: %d\n", id, pid);
        }
    }

    int pid = 0, status = 0;
    while (~(pid = wait(&status))) 
        printf("get child %d\n", pid);

    exit(0);
}