#ifndef BACKTRACE_DEBUG_H
#define BACKTRACE_DEBUG_H
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>

#define OUTPUT_FILE stderr

// static inline void bt_debug()
// {
// #define MAX_STACK_DEPTH 64
//     void* addrs[MAX_STACK_DEPTH];
//     int num = backtrace(addrs, MAX_STACK_DEPTH);

//     char** stack = backtrace_symbols(addrs, num);
    
//     fprintf(OUTPUT_FILE, "----backtrace begin----\n");
//     for (int i = 0; i < num; i++)
//         fprintf(OUTPUT_FILE, "%d: %s\n", i, stack[i]);
//     fprintf(OUTPUT_FILE, "----backtrace end----\n");

//     free(stack);
// }

static inline void print_stack(void* addrs, int num)
{
    char** stack = backtrace_symbols(addrs, num);
    
    fprintf(OUTPUT_FILE, "----backtrace begin----\n");
    for (int i = 0; i < num; i++)
        fprintf(OUTPUT_FILE, "%d: %s\n", i, stack[i]);
    fprintf(OUTPUT_FILE, "----backtrace end----\n");

    free(stack);
}

#define MAX_STACK_DEPTH 64

#define bt_debug() \
do \
{ \
    void* addrs[MAX_STACK_DEPTH]; \
    int num = backtrace(addrs, MAX_STACK_DEPTH); \
    print_stack(addrs, num); \
} while(0)

// static inline bt_debug()
// {
// #define MAX_STACK_DEPTH 64
//     void* addrs[MAX_STACK_DEPTH];
//     int num = backtrace(addrs, MAX_STACK_DEPTH);

//     print_stack(addrs, num);
// }



#endif // BACKTRACE_DEBUG_H 