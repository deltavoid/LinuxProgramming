/*
    echo server
    one connection per thread
*/
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#include "util.h"

const int buf_size = 4096;
const int backlog = 10;
const int max_conns = 4096;
int sock_fds[max_conns];
pthread_t sock_threads[max_conns];
int sock_num = 0;

void *worker(void *arg)
{
    int fd = *(int *)arg;
    printf("worker on %d\n", fd);

    char buf[buf_size];

    while (true)
    {
        int recv_len = recv(fd, buf, buf_size, 0);
        if (recv_len <= 0)
            break;
        printf("recv_len: %d\n", recv_len);

        int send_len = send_full(fd, buf, recv_len, 0);
    }

    close(fd);
    printf("connection %d closed\n", fd);
    pthread_exit(NULL);
}

void checksockname(int fd)
{
    struct sockaddr_in that_addr;
    const int addr_buf_size = 100;
    char addr_buf[addr_buf_size];

    int sin_size = sizeof(that_addr);
    if (getsockname(fd, (struct sockaddr *)&that_addr, (socklen_t *)&sin_size) != 0)
        perror("getsockname error");
    printf("getsockname: %s:%d\n", inet_ntop(AF_INET, &that_addr.sin_addr, addr_buf, addr_buf_size),
           ntohs(that_addr.sin_port));

    sin_size = sizeof(that_addr);
    if (getpeername(fd, (struct sockaddr *)&that_addr, (socklen_t *)&sin_size) != 0)
        perror("getpeername error");
    printf("getpeername: %s:%d\n", inet_ntop(AF_INET, &that_addr.sin_addr, addr_buf, addr_buf_size),
           ntohs(that_addr.sin_port));
}

int main(int argc, char** argv)
{
    if  (!(argc > 1))
    {   printf("usage: %s <port>\n", argv[0]);
        return 0;
    }

    struct sockaddr_in this_addr;
    this_addr.sin_family = AF_INET;
    this_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(this_addr.sin_zero), sizeof(this_addr.sin_zero));

    int port_;
    if  (sscanf(argv[1], "%d", &port_) == -1)  perror("bad port");
    this_addr.sin_port = htons((short)port_);


    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        perror("socket error");

    
    if (bind(fd, (struct sockaddr *)&this_addr, sizeof(struct sockaddr)) == -1)
        perror("bind error");

    if (listen(fd, backlog) == -1)
        perror("listen error");



    while (true)
    {
        struct sockaddr_in that_addr;
        int sin_size = sizeof(struct sockaddr_in);

        if ((sock_fds[sock_num] = accept(fd, (struct sockaddr *)&that_addr, (socklen_t *)&sin_size)) == -1)
            perror("accept error");

        printf("%d's connection %d from %s:%d\n", sock_num, sock_fds[sock_num],
               inet_ntoa(that_addr.sin_addr), ntohs(that_addr.sin_port));
        
        checksockname(sock_fds[sock_num]);

        if (pthread_create(&sock_threads[sock_num], NULL, worker, (void *)&sock_fds[sock_num]) == -1)
            perror("pthread error");

        sock_num++;
        if (sock_num >= max_conns)
            perror("too many connections");
    }

    return 0;
}
