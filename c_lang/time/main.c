#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <time.h>



int main()
{
    printf("hello world\n");



    time_t timer = time(NULL);
    printf("time is %ld, ctime is %s\n",timer, ctime(&timer)); //得到日历时间


    return 0;
}