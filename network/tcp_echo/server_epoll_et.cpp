/*
    echo server
    one loop per thread, epoll, reuseport
*/
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <map>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>

#include "util.h"

using namespace std;

const int max_threads = 8;
pthread_t threads[max_threads];
int listen_fds[max_threads];

const int buf_size = 4096;
const int backlog = 10;
const int max_events = 4096;

class Connection
{
public:
    // int fd;

    Connection()
    {
    }

    ~Connection()
    {
    }

    int handle(int fd, int event)
    {
    }
};

map<int, Connection> conns;

void handle_accpet(int listen_fd, int epoll_fd)
{
    struct sockaddr_in that_addr;
    int sin_size = sizeof(struct sockaddr_in);
    int fd = -1;
    if ((fd = accept4(listen_fd, (struct sockaddr *)&that_addr, (socklen_t *)&sin_size, SOCK_NONBLOCK)) == -1)
    {
        perror("accept error");
        return;
    }
    printf("establish connection on fd %d from %s:%d\n", fd, inet_ntoa(that_addr.sin_addr),
           ntohs(that_addr.sin_port));

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1)
        perror("epoll_ctl add error");
}

// this has not failure tolerrance
int handle_echo(int fd, char *buf)
{
    int recv_len = recv(fd, buf, buf_size, 0);
    // printf("recv %d bytes on fd %d\n", recv_len, fd);
    if (recv_len <= 0)
        return recv_len;

    int send_len = send(fd, buf, recv_len, 0);
    return send_len;
}

void close_fd(int fd, int epoll_fd)
{
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
        perror("epoll_ctl del error");
    close(fd);
    printf("fd %d is closed\n", fd);
}

// one loop per thread
void *loop(void *arg)
{
    int listen_fd = *(int *)arg;

    int epoll_fd = -1;
    if ((epoll_fd = epoll_create1(0)) == -1)
        perror("epoll_create1 error");

    printf("loop: listen_fd: %d, epoll_fd: %d\n", listen_fd, epoll_fd);

    struct epoll_event listen_event;
    listen_event.events = EPOLLIN;
    listen_event.data.fd = listen_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &listen_event) == -1)
        perror("epoll_ctl listen_fd error");

    struct epoll_event *events = new struct epoll_event[max_events];
    // memset(events, 0, sizeof(struct epoll_event) * max_events);
    char *echo_buf = new char[buf_size];

    if (listen(listen_fd, backlog) == -1)
        perror("listen error");

    while (true)
    {
        int num = -1;
        if ((num = epoll_wait(epoll_fd, events, max_events, -1)) == -1)
            perror("epoll_wait error");

        for (int i = 0; i < num; i++)
        {
            struct epoll_event &event = events[i];

            if (event.events | EPOLLIN)
            {
                if (event.data.fd == listen_fd)
                    handle_accpet(listen_fd, epoll_fd);
                else
                {

                    if (handle_echo(event.data.fd, echo_buf) <= 0)
                        close_fd(event.data.fd, epoll_fd);
                }
            }
            else
            {
                close_fd(event.data.fd, epoll_fd);
            }
        }
    }

    close(epoll_fd);
    close(listen_fd);
    delete[] events;
    delete[] echo_buf;
    printf("loop on fd %d exit\n", listen_fd);
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("usage: %s <port> [<threads>]\n", argv[0]);
        return 0;
    }

    int port = 0;
    if (sscanf(argv[1], "%d", &port) < 0)
        perror("bad port");

    int num_threads = 1;
    if (argc > 2)
        if (sscanf(argv[2], "%d", &num_threads) < 0)
            perror("bad threads");

    struct sockaddr_in this_addr;
    this_addr.sin_family = AF_INET;
    this_addr.sin_port = htons((short)port);
    this_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(this_addr.sin_zero), sizeof(this_addr.sin_zero));

    for (int i = 0; i < num_threads; i++)
    {
        int fd = -1;
        if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            perror("socket error");

        if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) < 0)
            perror("fcntl error");

        int reuse = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&reuse, sizeof(int)) < 0)
            perror("set reuseaddr error");

        if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (const void *)&reuse, sizeof(int)) < 0)
            perror("set reuseport error");

        if (bind(fd, (struct sockaddr *)&this_addr, sizeof(struct sockaddr)) == -1)
            perror("bind error");

        listen_fds[i] = fd;
        if (pthread_create(&threads[i], NULL, loop, (void *)&listen_fds[i]) < 0)
            perror("pthread_create error");
    }

    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
