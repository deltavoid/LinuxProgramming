#include <cstdio>
#include <cstdlib>
#include <cstring>



int main(int argc, char** argv)
{
    for (int i = 0; i < argc; i++)
    {
        printf("%s ", argv[i]);
    }
    printf("\n");

    return 0;
}