#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>







typedef long (*func_t)(long arg0, long arg1, long arg2,
        long arg3, long arg4, long arg5);


long func1(long arg0)
{
    printf("func1: %ld\n", arg0);
    return 1;
}

long func2(int arg0, int arg1)
{
    printf("func2: %ld, %ld\n", arg0, arg1);
    return 2;
}

long func6(long arg0, int arg1, int arg2,
        int arg3, int arg4, long arg5)
{
    printf("func6: %ld, %ld, %ld, %ld, %ld, %ld\n",
        arg0, arg1, arg2, arg3, arg4, arg5);
    return 6;
}

func_t func_array[4] = {
    (func_t)func1,
    (func_t)func2,
    (func_t)func6,
    NULL
};


long hook_func(long arg0, long arg1, long arg2,
        long arg3, long arg4, long arg5)
{
    for (int i = 0; i < 4; i++)
        if  (func_array[i])
        {
            long ret = func_array[i](arg0, arg1, arg2, arg3, arg4, arg5);
            printf("hook_func: func_ret: %ld\n", ret);
        }
}



int main()
{
    hook_func(1, 2, 3, 4, 5, 6);
    return 0;
}