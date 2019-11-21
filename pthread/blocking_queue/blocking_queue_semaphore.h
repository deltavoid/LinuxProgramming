#ifndef BLOCKING_QUEUE_H
#define BLOCKING_QUEUE_H
#include <queue>
#include <pthread.h>
#include <semaphore.h>


class BlockingQueue
{public:
    std::queue<void*> queue;
    pthread_mutex_t mutex;
    sem_t not_empty;

    BlockingQueue()
    {
        pthread_mutex_init(&mutex, NULL);
        sem_init(&not_empty, 0, 0);
    }

    ~BlockingQueue()
    {
        pthread_mutex_destroy(&mutex);
        sem_destroy(&not_empty);
    }

    void put(void* item)
    {
        pthread_mutex_lock(&mutex);
        
        queue.push(item);
        
        pthread_mutex_unlock(&mutex);
        sem_post(&not_empty);
    }

    void* take()
    {
        sem_wait(&not_empty);
        pthread_mutex_lock(&mutex);
        
        void* item = queue.front();
        queue.pop();

        pthread_mutex_unlock(&mutex);

        return item;
    }

    bool empty()
    {
        pthread_mutex_lock(&mutex);
        
        bool empty = queue.empty();
        
        pthread_mutex_unlock(&mutex);
        
        return empty;
    }

};


#endif