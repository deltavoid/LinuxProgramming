#include <stdio.h>
#include <stdlib.h>


struct data {
    int a;
    int b:4,
        c:4;
};


int main()
{
    struct data data = {.a = 1, .b = 2, .c = 3};
    printf("a: %d, b: %d, c: %d\n", data.a, data.b, data.c);
    // compilition error
    // printf("a_addr: %p, b_addr: %p, c_addr: %p", &data.a, &data.b, &data.c);

    return 0;
}