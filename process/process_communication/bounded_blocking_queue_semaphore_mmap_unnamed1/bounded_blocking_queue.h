#ifndef BOUNDED_BLOCKING_QUEUE_H
#define BOUNDED_BLOCKING_QUEUE_H
#include <semaphore.h>


// const int queue_size = 1000;

struct BoundedBlockingQueue
{
    sem_t mutex;
    sem_t empty, full;
    int item_size, queue_size;
    int f, p;  // the queue has use limit of int32.
    char que[];
};

// the init and destroy does not alloc or free memory
extern void bbq_init(struct BoundedBlockingQueue* bbq, int item_size, int queue_size);
extern void bbq_destroy(struct BoundedBlockingQueue* bbq);
extern bool bbq_put(struct BoundedBlockingQueue* bbq, void* item);
extern bool bbq_take(struct BoundedBlockingQueue* bbq, void* item);
extern bool bbq_empty(struct BoundedBlockingQueue* bbq);


#endif