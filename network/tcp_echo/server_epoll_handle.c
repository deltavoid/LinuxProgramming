/*
    echo server
    one loop per thread, epoll
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>


#define buf_size 4096
#define backlog 10
#define max_events 4096



typedef int (*epoll_handler_t)(void* data, uint32_t events);
typedef void (*destructor_t)(void* data);

struct EpollHandler
{
    epoll_handler_t handler;
    destructor_t destructor;
};

inline int epoll_handle(void* data, uint32_t events)
{
    return (*(((EpollHandler*)data)->handler))(data, events);
}

inline void epoll_destruct(void* data)
{
    (*(((EpollHandler*)data)->destructor))(data);
}



struct Connection
{
    EpollHandler epoll_handler;
    char* buf;
    int fd;
    int epoll_fd;
};

int connection_echo(struct Connection* conn)
{
    int fd = conn->fd;
    char* buf = conn->buf;

        int recv_len = recv(fd, buf, buf_size, 0);
        // printf("recv %d bytes on fd %d\n", recv_len, fd);
        if  (recv_len <= 0)  return recv_len;

        int send_len = send(fd, buf, recv_len, 0);
        return send_len;

}

// this is the main interface for epoll users.
int connection_handler(void* data, uint32_t events)
{
    struct Connection* conn = (struct Connection*)data;
    int ret = 0;

    if  (events & EPOLLIN)
    {
        if  (connection_echo(conn) <= 0)
            ret = -1; 
    }
    else
        ret = -1;

    return ret;
}

void connection_destructor(void* data)
{
    struct Connection* conn = (struct Connection*)data;
    int fd = conn->fd;
    int epoll_fd = conn->epoll_fd;

    if  (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) < 0)
    {   perror("epoll_ctl_del error");
    }

    close(fd);

    free(data);

    printf("connection_destructor: fd %d is closed\n", fd);
}

struct Connection* connection_constructor(int fd, int epoll_fd, char* buf)
{
    struct Connection* conn = (struct Connection*)malloc(sizeof(struct Connection));

    conn->epoll_handler.handler = connection_handler;
    conn->epoll_handler.destructor = connection_destructor;

    conn->fd = fd;
    conn->epoll_fd = epoll_fd;
    conn->buf = buf;

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.ptr = conn;
    if  (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0)
    {   perror("epoll_ctl_add error");
        return NULL;
    }

    return conn;
}


void handle_accept(int listen_fd, int epoll_fd,  char* buf)
{
    struct sockaddr_in that_addr;
    int sin_size = sizeof(struct sockaddr_in);
    int fd = -1;
    if  ((fd = accept4(listen_fd, (struct sockaddr*)&that_addr, (socklen_t*)&sin_size, SOCK_NONBLOCK)) == -1)  
    {   perror("accept error");
        return;
    }
    printf("establish connection on fd %d form %s:%d\n", fd, inet_ntoa(that_addr.sin_addr), 
            ntohs(that_addr.sin_port));


    struct Connection* conn = connection_constructor(fd, epoll_fd, buf);
}



// one loop per thread
void* loop(void* arg)
{
    int listen_fd  = *(int*)arg;
    printf("loop on fd %d\n", listen_fd);


    int epoll_fd = -1;
    if  ((epoll_fd = epoll_create1(0)) == -1)  perror("epoll_create1 error");

    struct epoll_event listen_event;
    listen_event.events = EPOLLIN;
    // listen_event.data.fd = listen_fd;
    listen_event.data.ptr = NULL;
    if  (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &listen_event) == -1)  perror("epoll_ctl listen_fd error");

    struct epoll_event* events = (struct epoll_event*)malloc(sizeof(struct epoll_event) * max_events);
    char* echo_buf = (char*)malloc(sizeof(char) * buf_size);


    if  (listen(listen_fd, backlog) == -1)  perror("listen error");

    while (true)
    {
        int num = -1;
        if  ((num = epoll_wait(epoll_fd, events, max_events, -1)) == -1)  perror("epoll_wait error");

        for (int i = 0; i < num; i++)
        {   uint32_t evs = events[i].events;
            void* data = events[i].data.ptr;

            // if  (fd == listen_fd)
            if  (data == NULL)
            {
                handle_accept(listen_fd, epoll_fd, echo_buf);
            }
            else
            {
                // use int handle(void* data, uint32_t events) as interface between epoll and concret handler, e.g. connection.
                if  (epoll_handle(data, evs) < 0)
                    epoll_destruct(data);
            }
        }
    }

    close(epoll_fd);
    close(listen_fd);
    free(events);
    free(echo_buf);
    printf("loop on fd %d exit\n", listen_fd);
    return NULL;
}


int main(int argc, char** argv)
{
    if  (argc < 2)
    {   printf("usage: %s <port>\n", argv[0]);
        return 0;
    }

    int port = 0;
    if  (sscanf(argv[1], "%d", &port) < 0)  perror("bad port");
    
    struct sockaddr_in this_addr;
    this_addr.sin_family = AF_INET;
    this_addr.sin_port = htons((short)port);
    this_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(this_addr.sin_zero), sizeof(this_addr.sin_zero));


    int fd = -1;
    if  ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)  perror("socket error");

    if  (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) < 0) perror("fcntl error");  

    if  (bind(fd, (struct sockaddr*)&this_addr, sizeof(struct sockaddr)) == -1)  perror("bind error");


    loop(&fd);



    return 0;
}
