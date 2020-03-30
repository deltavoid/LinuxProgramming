#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>


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
    int fd;
    int len;

    fd = open(path, O_RDONLY);
    if  (fd < 0) perror("open");
    
    len = read(fd, buf, BUF_LEN);
    if  (len < 0) perror("read");
    
    buf[len] = '\0';
    printf("%s", buf);
    
    close(fd);
}

void write_(const char* path)
{
    int fd;
    int len;
    char str[] = "hello world\n";

    //fd2 = open("file2.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    // fd = open(path, O_WRONLY | O_CREAT, 0644);
    if  (fd < 0) perror("open");
    
    // buf = "hello world!";
    // len = write(fd, "hello world", strlen("hello world"));
    len = write(fd, str, strlen(str));

    close(fd);
}


int main(int argc, char** argv)
{

    show_arg(argc, argv);
    show_cwd();
    
    write_(file_path);
    read_(file_path);

    return 0;
}