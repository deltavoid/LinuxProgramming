#ifndef BLOCKING_QUEUE_H
#define BLOCKING_QUEUE_H
#include <queue>
#include <pthread.h>


class BlockingQueue
{public:
    std::queue<void*> queue;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;

    BlockingQueue()
    {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&not_empty, NULL);
    }

    ~BlockingQueue()
    {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&not_empty);
    }

    void put(void* item)
    {
        pthread_mutex_lock(&mutex);
        
        queue.push(item);

        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&not_empty);
    }

    void* take()
    {
        pthread_mutex_lock(&mutex);
        while (queue.empty())
            pthread_cond_wait(&not_empty, &mutex);
        
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