#include <stdio.h>
#include <stdlib.h>

#include "backtrace_debug.h"




int dfs(int k, int n)
{
    if  (k > n)
    {
        // printf("%d\n", k);
        // struct backtrace_state state;
        // backtrace_print(NULL, 0, stdout);
        bt_debug();
    }
    else
        dfs(k + 1, n);
}


int main()
{
    dfs(1, 3);

    return 0;
}