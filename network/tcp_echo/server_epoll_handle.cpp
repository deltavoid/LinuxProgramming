/*
    echo server
    one loop per thread, epoll
*/
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <unordered_map>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>



const int buf_size = 4096;
const int backlog = 10;
const int max_events = 4096;



class EpollHandler
{public:
    virtual int handle(uint32_t events) { return -1;}
    virtual ~EpollHandler() {}
};



class Connection : public EpollHandler
{public:
    char* buf;
    int fd;
    int epoll_fd;


    Connection(int fd, int epoll_fd, char* buf) : fd(fd), epoll_fd(epoll_fd), buf(buf) 
    {
        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.ptr = this;

        if  (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0)
        {   perror("epoll_ctl_add error");
        }
    }

    virtual ~Connection() 
    {
        if  (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) < 0)
            perror("epoll_ctl_del error");
        
        close(fd);
        printf("fd %d is closed\n", fd);
    }


    // this has not failure tolerrance
    int echo()
    {
        int recv_len = recv(fd, buf, buf_size, 0);
        // printf("recv %d bytes on fd %d\n", recv_len, fd);
        if  (recv_len <= 0)  return recv_len;

        int send_len = send(fd, buf, recv_len, 0);
        return send_len;
    }

    // this is the main interface for epoll users.
    virtual int handle(uint32_t events)
    {
        int ret = 0;

        if  (events & EPOLLIN)
        {
            if  (echo() <= 0)
                ret = -1; 
        }
        else
            ret = -1;

        return ret;
    }
};



void handle_accept(int listen_fd, int epoll_fd, char* buf)
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

    Connection* conn = new Connection(fd, epoll_fd, buf);
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

    struct epoll_event* events = new struct epoll_event[max_events];
    // memset(events, 0, sizeof(struct epoll_event) * max_events);
    char* echo_buf = new char[buf_size];


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
                // put epoll_ctl_del(...) to Connection is a choice worth consideration.
                struct EpollHandler* epoll_handler = (struct EpollHandler*)data;

                if  (epoll_handler->handle(evs) < 0)
                    delete epoll_handler;
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
