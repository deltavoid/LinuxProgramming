#include <cstdio>
#include <cstdlib>
#include <queue>
#include <unistd.h>
#include <pthread.h>
// #include "blocking_queue_condvar.h"
#include "blocking_queue_semaphore.h"


typedef long long ll;


const ll producer_num = 2;
const ll consumer_num = 2;
pthread_t producer_threads[producer_num];
pthread_t consumer_threads[consumer_num];

struct Node
{
    ll producer_id;
    ll data;

    Node(ll id, ll data): producer_id(id), data(data) {}
};

BlockingQueue que;
const ll item_num = 1000;



void* producer(void* arg)
{
    ll id = (ll)arg;
    printf("producer %lld\n", id);

    Node* item = NULL;
    for (ll i = 0; i <= item_num; i++)
    {
        if  (i == item_num)  item = NULL;
        else item = new Node(id, i);

        que.put((void*) item);

        sleep(1);
    }


    pthread_exit(NULL);
}

void* consumer(void* arg)
{
    ll id = (ll)arg;
    printf("consumer %lld\n", id);

    Node* item = NULL;
    while ((item = (Node*)que.take()) != NULL)
    {
        
        printf("producer: %lld consumer: %lld data: %lld\n",
                item->producer_id, id, item->data);
        
        sleep(1);
    }


    pthread_exit(NULL);
}

int main()
{
    for (ll i = 0; i < producer_num; i++)
    {   int ret = pthread_create(&producer_threads[i], NULL, producer, (void*)i);
    }
    for (ll i = 0; i < consumer_num; i++)
    {   int ret = pthread_create(&consumer_threads[i], NULL, consumer, (void*)i);
    }



    for (ll i = 0; i < producer_num; i++)
    {   int ret = pthread_join(producer_threads[i], NULL);
    }
    for (ll i = 0; i < consumer_num; i++)
    {   int ret = pthread_join(consumer_threads[i], NULL);
    }

    printf("hello world\n");
    return 0;
}