#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>  /* Definition of uint64_t */

#include <unistd.h>
#include <sys/eventfd.h>

 
// #define handle_error(msg) \
//    do { perror(msg); exit(EXIT_FAILURE); } while (0)

static inline void handle_error(const char* msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}
 
// 目前已知eventfd 只能用于父子进程和线程内，
// 在不使用父子进程的情况下，更多用于进程内线程间通信，
// 且通常挂在epoll上用于唤醒线程，
int main(int argc, char *argv[])
{
   uint64_t u;
	
   int efd = eventfd(1, 0);
   if (efd == -1)  handle_error("eventfd");
   
    int ret = fork();
    if(ret == 0)
    {
        for (int j = 1; j < argc; j++) {
            printf("Child writing %s to efd\n", argv[j]);
            u = atoll(argv[j]);
         
            ssize_t s = write(efd, &u, sizeof(uint64_t));
            if (s != sizeof(uint64_t))  handle_error("write");
            
            sleep(2);
       }
       printf("Child completed write loop\n");
 
       exit(EXIT_SUCCESS);
    }
    else
    {
        for (int j = 0; j < argc; j++)
        {
            ssize_t s = read(efd, &u, sizeof(uint64_t));
            if  (s != sizeof(uint64_t))     handle_error("read");
            printf("Parent read %llu from efd\n",(unsigned long long)u);

            // sleep(1);
        }

        exit(EXIT_SUCCESS);
    }
}
