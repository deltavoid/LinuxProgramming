#include <stdio.h>
#include <stdlib.h>

#include "logging.h"


void dfs(int k, int n)
{
    if  (k > n)
        bt_debug();
    else
        dfs(k + 1, n);
}

int main()
{
    fprintf(stderr, "hello world\n");

    LOG_DEBUG("hello debug: %d\n", 1);
    LOG_INFO("hello info\n");
    LOG_WARN("hello warn\n");
    LOG_ERROR("hello error\n");
    LOG_FATAL("hello fatal\n");

    dfs(1, 3);

    return 0;
}