#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>



typedef long (*hello_func_t)(long arg0, long arg1, long arg2);

hello_func_t hello_addr;

long hello_func(long arg0, long arg1, long arg2)
{
    return (arg0 + arg1 + arg2);
}


long hello_func1(long arg0, long arg1, long arg2)
{
    return (arg0 + arg1 + arg2) + 1;
}




typedef long (*func_t)(long arg0, long arg1, long arg2,
        long arg3, long arg4, long arg5);



struct hook_pt_regs {
    long args[6];
    long ret;
    long blank;
};

typedef long (*post_func_t)(struct hook_pt_regs* ctx);




#define HOOK_ENABLE_POST_RUN 0x1

struct hook_func_t {
    unsigned long func;
    unsigned long flag;
};

struct hook_t {
    struct hook_func_t prev_func[8];
    struct hook_func_t post_func[8];
};




struct hook_t hello_hook;
bool hello_hook_inited = false;


long hello_hook_func(long arg0, long arg1, long arg2,
        long arg3, long arg4, long arg5)
{
    long ret;

    for (int i = 0; i < 8; i++)
        if  (hello_hook.prev_func[i].func)
        {
            ret = ((func_t)(hello_hook.prev_func[i].func))(arg0, arg1, arg2,
                arg3, arg4, arg5);

            if  (!(hello_hook.prev_func[i].flag & HOOK_ENABLE_POST_RUN))
                break;
        }

    struct hook_pt_regs ctx = {
        .args = {arg0, arg1, arg2, arg3, arg4, arg5},
        .ret = ret,
        .blank = 0
    };

    for (int i = 0; i < 8; i++)
        if  (hello_hook.post_func[i].func)
        {
            ((post_func_t)(hello_hook.post_func[i].func))(&ctx);
            
            if  (!(hello_hook.post_func[i].flag & HOOK_ENABLE_POST_RUN))
                break;
        }

    return ret;
}





// now only for hello_hook
int hello_register_hook(unsigned long func_addr, unsigned long flag, int index)
{
    if  (!hello_hook_inited)
    {
        struct hook_func_t origin_func_hook = {
            .func = (unsigned long)hello_func,
            .flag = 0
        };
        hello_hook.prev_func[7] = origin_func_hook;
        *(unsigned long*)&hello_addr = (unsigned long)hello_hook_func;
 
        hello_hook_inited = true;
    }

    if  (index == 7)  return -1;


    struct hook_func_t func_hook = {
        .func = func_addr,
        .flag = flag
    };

    if  (index < 8)
    {   
        hello_hook.prev_func[index] = func_hook;
    }
    else
    {
        index -= 8;
        hello_hook.post_func[index] = func_hook;
    }

    return 0;
}




int main()
{
    *(unsigned long*)&hello_addr = (unsigned long)hello_func;
    long ret = hello_addr(1, 2, 3);
    printf("ret: %ld\n", ret);

    hello_register_hook((unsigned long)hello_func1, 1, 0);
    long ret1 = hello_addr(1, 2, 3);
    printf("ret1: %ld\n", ret1);


   return 0;
}