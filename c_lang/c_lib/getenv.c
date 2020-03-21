#include <stdio.h>
#include <stdlib.h>

const char name0[] = "PATH";
const char name1[] = "HELLO";


void display_env(const char* name)
{
    char* val = getenv(name);

    printf("%s: %s\n", name, val);
}

int main()
{
    display_env(name0);
    display_env(name1);
    

    return 0;
}