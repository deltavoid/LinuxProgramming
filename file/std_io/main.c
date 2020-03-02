#include <stdio.h>
#include <string.h>

#include <unistd.h>


const char file_path[] = "file.txt";
// const int buf_len = 100;
#define BUF_LEN 1000
char buf[BUF_LEN];


void show_arg(int argc, char** argv)
{
    int i;
    for(i = 0; i < argc; i++)
        printf("%s ", argv[i]);
    printf("\n");
}

void show_cwd()
{
    char* cwd = getcwd(buf, BUF_LEN);
    printf("cwd: %s\n", cwd);
}

void read_(const char* path)
{
    FILE* file;
    int len;

    file = fopen(path, "r");
    if  (!file)  perror("open error");

    len = fread(buf, 1, BUF_LEN, file);
    printf("read: %d\n", len);
    if  (len < 0)  perror("read error"); 

    buf[len] = '\0';
    printf("%s", buf);

    fclose(file);
}

void write_(const char* path)
{
    FILE* file;
    int len;
    char str[] = "hello world\n";

    // file = fopen(path, "w+");
    file = fopen(path, "w");
    if  (!file)  perror("open error");

    len = fwrite(str, 1,  strlen(str), file);
    printf("write: %d\n", len);
    if  (len < 0)  perror("write error");

    fclose(file);
}


int main(int argc, char** argv)
{

    show_arg(argc, argv);
    show_cwd();
    
    write_(file_path);
    read_(file_path);

    return 0;
}