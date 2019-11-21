#include "bounded_blocking_queue.h"
#include <cstdio>
#include <cstring>

// the init and destroy does not alloc or free memory
void bbq_init(struct BoundedBlockingQueue* bbq, int item_size, int queue_size)
{
    bbq->f = 0;  
    bbq->p = 0;
    bbq->item_size = item_size;
    bbq->queue_size = queue_size;
    sem_init(&bbq->mutex, 1, 1);
    sem_init(&bbq->empty, 1, queue_size);
    sem_init(&bbq->full, 1, 0);
    // memset(bbq->que, 0, item_size * queue_size);
}

void bbq_destroy(struct BoundedBlockingQueue* bbq)
{
    sem_destroy(&bbq->mutex);
    sem_destroy(&bbq->empty);
    sem_destroy(&bbq->full);
}

bool bbq_put(struct BoundedBlockingQueue* bbq, void* item)
{
    // printf("put\n");
    sem_wait(&bbq->empty);
    sem_wait(&bbq->mutex);

    // bbq->que[(bbq->p++) % queue_size] = item;
    char* pointer = bbq->que + bbq->item_size * (bbq->p++ % bbq->queue_size);
    memcpy(pointer, item, bbq->item_size);

    sem_post(&bbq->mutex);
    sem_post(&bbq->full);
    // printf("put end\n");

    return true;
}

bool bbq_take(struct BoundedBlockingQueue* bbq, void* item)
{
    // printf("take\n");
    sem_wait(&bbq->full);
    sem_wait(&bbq->mutex);
    
    // void* item = bbq->que[(bbq->f++) % queue_size];
    char* pointer = bbq->que + bbq->item_size * (bbq->f++ % bbq->queue_size);
    memcpy(item, pointer, bbq->item_size);

    sem_post(&bbq->mutex);
    sem_post(&bbq->empty);
    // printf("take end\n");

    return true;
}

bool bbq_empty(struct BoundedBlockingQueue* bbq)
{
    sem_wait(&bbq->mutex);

    bool empty = (bbq->f == bbq->p);

    sem_post(&bbq->mutex);

    return empty;
}
