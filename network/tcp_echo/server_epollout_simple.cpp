/*
    echo server
    one loop per thread, epoll
*/
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <string>
#include <unordered_map>
#include <algorithm>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>

// #include "util.h"

const int buf_size = 4096;
const int backlog = 10;
const int max_events = 4096;

const int reponse_size = 1024 * 1024 * 1024; // 1G
char reponse[reponse_size];

// send a reponse upon connection establish, and then close
class Connection
{
public:
    int fd;
    int epoll_fd;
    // char buf[buf_size];
    // int reponse_num;
    int recv_len;
    int send_len;
    bool enable_epollout;
    static int send_cnt;

    Connection(int fd, int epoll_fd) : fd(fd), epoll_fd(epoll_fd), recv_len(0)
    {
        enable_epollout = false;
        send_len = 0;
        send_buf();
    }
    ~Connection() {}

    void set_epollout(bool flag)
    {
        struct epoll_event event;
        event.events = EPOLLIN | (flag? EPOLLOUT : 0);
        event.data.fd = fd;
        if  (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event) < 0)
            perror("epoll_ctl error");
        enable_epollout = flag;
    }

    // this function has two entries, the user and epollout.
    int send_buf()
    {
        int tmp = 0;
        int last_send_len = send_len;

        // while 
        if  
            (send_len < reponse_size && (tmp = ::send(fd, reponse + send_len, reponse_size - send_len, 0)) > 0)
        // if  (send_len < reponse_size && (tmp = ::send(fd, reponse + send_len, std::min(reponse_size - send_len, 64), 0)) > 0)
        {
            send_len += tmp;
        }

        // usleep(1000);

        if (send_len == reponse_size)
        {
            if  (enable_epollout)
                set_epollout(false);
            return 1;
        }

        if  (tmp >= 0)
            return 0;
            
        if (tmp < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        {
            if  (!enable_epollout)
                set_epollout(true);
            return 0;
        }

        // if (tmp < 0)
        { // handle error
            return -1;
        }
    }

    // this has not failure tolerrance
    int handle_recv(char *buf)
    {
        printf("handle_recv: 1, fd: %d\n", fd);
        // int recv_len = recv(fd, buf, buf_size, 0);
        // // printf("recv %d bytes on fd %d\n", recv_len, fd);
        // if (recv_len <= 0)
        //     return -1;

        // this->recv_len += recv_len;
        // return 0;

        int tmp = 0;
        // this need not while.
        while ((tmp = ::recv(fd, buf, buf_size, 0)) > 0)
        {   recv_len += tmp;
        }

        if (tmp < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            return 0;
        
        // if  (tmp <= 0)
        // handle error
            return -1;
    }

    int handle_send()
    {
        
        // printf("handle_send: 1, fd: %d, send_cnt: %d\n", fd, send_cnt++);

        int ret = send_buf();
        return ret == 0? 0 : -1;
    }

    int handle(int events, char *buf)
    {
        // int ret = 0;

        // if  (events & EPOLLERR)
        //     goto error;
        if (events & ~(EPOLLIN | EPOLLOUT))
        {
            printf("fd %d recv unexpected events: %d %s EPOLLERR\n", fd, events,
                   (events & EPOLLERR) ? "including" : "excluding");
            goto error;
        }

        if (events & EPOLLOUT)
            if (handle_send() < 0)
                goto error;

        if (events & EPOLLIN)
            if (handle_recv(buf) < 0)
                goto error;

        return 0;
    error:
        return -1;
    }
};

int Connection::send_cnt = 0;

typedef std::unordered_map<int, Connection *> ConnectionMap;
ConnectionMap conns;

void handle_accept(int listen_fd, int epoll_fd, ConnectionMap &conns)
{
    struct sockaddr_in that_addr;
    int sin_size = sizeof(struct sockaddr_in);
    int fd = -1;
    if ((fd = accept4(listen_fd, (struct sockaddr *)&that_addr, (socklen_t *)&sin_size, SOCK_NONBLOCK)) == -1)
    {
        perror("accept error");
        return;
    }
    printf("establish connection on fd %d form %s:%d\n", fd, inet_ntoa(that_addr.sin_addr),
           ntohs(that_addr.sin_port));

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLOUT;
    event.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1)
        perror("epoll_ctl add error");

    Connection *conn = new Connection(fd, epoll_fd);
    conns.insert(std::make_pair(fd, conn));
}

void close_conn(int fd, int epoll_fd, ConnectionMap &conns, ConnectionMap::iterator &it)
{
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
        perror("epoll_ctl del error");

    delete it->second;
    conns.erase(it);

    close(fd);
    printf("fd %d is closed\n", fd);
}

// one loop per thread
void *loop(void *arg)
{
    int listen_fd = *(int *)arg;
    printf("loop on fd %d\n", listen_fd);

    int epoll_fd = -1;
    if ((epoll_fd = epoll_create1(0)) == -1)
        perror("epoll_create1 error");

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
        // printf("epoll_wait ret %d\n", num);

        for (int i = 0; i < num; i++)
        {
            struct epoll_event &event = events[i];
            int fd = events[i].data.fd;

            if (fd == listen_fd)
            {
                handle_accept(listen_fd, epoll_fd, conns);
            }
            else
            {
                ConnectionMap::iterator it = conns.find(fd);
                if (it != conns.end())
                {
                    // use handle(events) as interface between eventloop and concrete object.
                    int ret = it->second->handle(event.events, echo_buf);
                    // printf("fd: %d, ret: %d\n", fd, ret);
                    if (ret < 0)
                        close_conn(fd, epoll_fd, conns, it);
                }
                else
                    printf("fd not found\n");
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
        printf("usage: %s <port>\n", argv[0]);
        return 0;
    }

    int port = 0;
    if (sscanf(argv[1], "%d", &port) < 0)
        perror("bad port");

    struct sockaddr_in this_addr;
    this_addr.sin_family = AF_INET;
    this_addr.sin_port = htons((short)port);
    this_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(this_addr.sin_zero), sizeof(this_addr.sin_zero));

    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        perror("socket error");

    if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) < 0)
        perror("fcntl error");

    if (bind(fd, (struct sockaddr *)&this_addr, sizeof(struct sockaddr)) == -1)
        perror("bind error");

    int64_t* length = (int64_t*)reponse;
    *length = reponse_size;
    for (int i = sizeof(int64_t); i < reponse_size; i+= sizeof(int64_t))
        *(int64_t*)(reponse + i) = 0x1234567812345678ll;

    loop(&fd);

    return 0;
}
