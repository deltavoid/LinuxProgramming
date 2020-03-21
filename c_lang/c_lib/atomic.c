#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <pthread.h>

typedef long long ll;



static volatile ll cnt;
pthread_t thread;


void* run(void* arg)
{
    ll id = (ll)arg;
    printf("run: %lld\n", id);

    for (int i = 0; i < 10; i++)
    {
        int ret = __sync_fetch_and_add(&cnt, 1);
        printf("id: %lld ret: %lld\n", id, ret);

        // sleep(1);
    }

    pthread_exit(NULL);
}

int main()
{
    int ret = 0;


    ret = pthread_create(&thread, NULL, run, (void*)1);

    run((void*)0);


    ret = pthread_join(thread, NULL);

    return 0;
}