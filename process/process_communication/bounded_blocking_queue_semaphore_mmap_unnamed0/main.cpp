#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>


#define CONSUMER_NUM 4
#define PRODUCER_NUM 4
#define QUEUE_LEN 100
#define TOTAL_LEN 100

struct Node
{   int id;
    int author;
    int value;
};


struct Shared
{   struct Node queue[QUEUE_LEN];
    int f, p;
    sem_t mutex, empty, full;//
};


struct Shared* s;


void producer_proc()
{
    struct Node tmp;
    int flag = 1;
    int addr;
    pid_t pid = getpid();

    printf("producer %d\n", pid);

    while (flag)
    {
        tmp.author = pid;
        tmp.value = rand();

        sem_wait(&s->empty);
        sem_wait(&s->mutex);

        addr = s->p % QUEUE_LEN;
        tmp.id = s->p;
        s->queue[addr] = tmp;
        s->p++;
        if  (s->p > TOTAL_LEN)  flag = 0;

        // sleep(0.1);
        sem_post(&s->mutex);
        sem_post(&s->full);
        sleep(1);
    }
}

void consumer_proc()
{
    struct Node tmp;
    int flag = 1;
    int addr;
    pid_t pid = getpid();

    printf("consumer %d\n", pid);

    while (flag)
    {
        sem_wait(&s->full);
        sem_wait(&s->mutex);

        addr = s->f % QUEUE_LEN;
        tmp = s->queue[addr];
        s->f++;
        if  (s->f > TOTAL_LEN)  flag = 0;

        // sleep(0.1);
        sem_post(&s->mutex);
        sem_post(&s->empty);
        // sleep(0.1);

        printf("consumer id:%d  product id:%d  author id:%d  value:%d\n",
               pid, tmp.id, tmp.author, tmp.value);
    }
}


//进程管理
//共享内存
//信号量



int main()
{
    int proc_flag = 0;
    int fd;
    char buf;
    pid_t pid;
    int status;
    int ret;
    int i;
    

    fd = open("share.txt", O_RDWR | O_CREAT, S_IRWXU);
    // for (i = 0; i < sizeof(struct Shared); i++)  write(fd, &buf, sizeof(buf));
    ftruncate(fd, sizeof(struct Shared)); 
    s = (Shared*)mmap(NULL, sizeof(struct Shared), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    s->f = 0;  s->p = 0;

    sem_init(&s->mutex, 1, 1);
    sem_init(&s->empty, 1, QUEUE_LEN);
    sem_init(&s->full, 1, 0);

    
    for (i = 0; i < PRODUCER_NUM + CONSUMER_NUM; i++)
    {   if  (fork() == 0)
        {   proc_flag = i < PRODUCER_NUM? 1 : 2;
            break;
        }
    }


    if  (proc_flag == 1)
    {   producer_proc();
        return 0;
    }
    else if  (proc_flag == 2)
    {   consumer_proc();
        return 0;
    }

    while (~(pid = wait(&status))) ;
    printf("parent\n");

    return 0;
}