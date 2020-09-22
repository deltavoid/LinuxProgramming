#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/file.h>
#include <sys/fcntl.h>

#include "../kmod/proc_entry_hashmap.h"



int main()
{
    int fd = open("/proc/example/hello", O_RDWR);

    struct hello_entry_param param = {
        .operation = HELLO_ENTRY_INSERT,
        .key = 1,
        .value = 1,
    };

    // for (int i = 0; i < 3; i++)
    if  (write(fd, &param, sizeof(param)) < 0)
    {
        perror("insert error");
        return -1;
    }

    param.operation = HELLO_ENTRY_GET;
    param.value = 0;

    // for (int i = 0; i < 3; i++)
    if  (read(fd, &param, sizeof(param)) < 0)
    {
        perror("get error");
        return -1;
    }
    printf("read key: %lld, vlaue: %lld\n", param.key, param.value);

    param.operation = HELLO_ENTRY_REMOVE;
    if  (write(fd, &param, sizeof(param)) < 0)
    {
        perror("remove error");
        return -1;
    }


    return 0;
}