#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <unistd.h>
#include <pthread.h>


// void* run(void* arg)
// {
//     printf("run: 1\n");

//     sleep(10000);

//     return NULL;
// }



int main()
{
    printf("main: 1\n");

    // pthread_t thread;
    // {
    //     int ret = pthread_create(&thread, NULL, run, NULL);
    // }

    // printf("main: 2\n");

    // {
    //     int ret = pthread_join(thread, NULL);
    // }

    // printf("main: 3\n");


    // printf("hello world\n");

    sleep(10000);

    return 0;
}