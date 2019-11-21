#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
// #include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "bounded_blocking_queue.h"


struct Node
{   int author;
    int id;
    Node (int author, int id) : author(author), id(id) {}
};


const int producer_num = 4;
const int consumer_num = 4;
const int queue_size = 1000;
const int item_num = 100;

struct BoundedBlockingQueue* bbq;


void producer_proc(int id)
{
    printf("producer: %d\n", id);

    for (int i = 0; i < item_num; i++)
    {
        Node node(id, i);
        printf("producer: %d, id: %d\n", id, i);
        
        bbq_put(bbq, &node);

        sleep(1);
    }
    Node node(-1, 0);
    bbq_put(bbq, &node);


    printf("producer %d end\n", id);
}

void consumer_proc(int id)
{
    printf("consumer: %d\n", id);

    Node node(-1, 0);
    while (bbq_take(bbq, &node) && node.author != -1)
    {
        printf("producer: %d, id: %d consumer: %d\n", node.author, node.id, id);

        // sleep(1);
    }

    printf("consumer %d end\n", id);
}


//进程管理
//共享内存
//信号量

/*
    fork, wait,
    mmap, anonymous
    semaphore
    BoundedBlockingQueue Interface and Implementation
    C style Object Programming, variable length object
    blocked communication style, stop-object used
*/


int main()
{
    printf("sem_size: %d\n", sizeof(sem_t));
    printf("bbq_size: %d\n", sizeof(BoundedBlockingQueue));
    // addr = mmap(addr, size, prot, flags, fd, offset);
    // shared objects should be copied to the shared queue.
    bbq = (struct BoundedBlockingQueue*)mmap(NULL, sizeof(BoundedBlockingQueue) + sizeof(Node) * queue_size, 
            PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    bbq_init(bbq, sizeof(Node), queue_size);


    int id;
    /*  [0, producer_num) : producer
        [producer_num, producer_num + consumer_num) : consumer
        producer_num + consumer : parent
    */
    for (id = 0; id < producer_num + consumer_num; id++)
    {   if  (fork() == 0)  
            break;
    }

    if  (id < producer_num)
    {   producer_proc(id);
        return 0;
    }
    else if  (id < producer_num + consumer_num)
    {   consumer_proc(id - producer_num);
        return 0;
    }


    printf("parent\n");
    int pid = 0, status = 0;    
    while (~(pid = wait(&status))) ;

    bbq_destroy(bbq);

    return 0;
}