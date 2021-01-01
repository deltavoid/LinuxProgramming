#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <time.h>



static inline uint64_t get_msecs(void) {
    int ret;
    struct timespec ts;

    ret = clock_gettime(CLOCK_MONOTONIC, &ts);
    if (ret != 0) {
        perror("flextcp get_msecs: clock_gettime failed\n");
        abort();
    }

    return ts.tv_sec * 1000ULL + (ts.tv_nsec / 1000000ULL);
}



int main()
{
    printf("hello world\n");



    time_t timer = time(NULL);
    printf("time is %ld, ctime is %s\n",timer, ctime(&timer)); //得到日历时间


    printf("msecs: %lld\n", get_msecs());


    return 0;
}