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
// #include <sys/epoll.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>



const int buf_size = 4096;
const int backlog = 10;
const int max_events = 4096;




class Poller
{public:
    static const int max_fds = 4096;
    static const int fd_range = 4096;

    struct pollfd pfds[max_fds];
    void* handlers[fd_range];
    int num;

    Poller()
    {
        num = 0;
        memset(pfds, 0, sizeof(pfds));
        memset(handlers, 0, sizeof(handlers));
    }

    ~Poller()
    {
    }


    int add_fd(int fd, short events, void* handler)
    {
        printf("Poller::add_fd: 1, fd: %d, events: %d, handler: %lx\n",
                fd, events, (unsigned long)handler);

        if  (fd >= fd_range)
            return -1;
        
        for (int i = 0; i < num; i++)
            if  (pfds[i].fd == fd)
                return -1;
        
        pfds[num].fd = fd;
        pfds[num].events = events;
        pfds[num].revents = 0;
        num++;
        handlers[fd] = handler;

        show_fds();
        return 0;
    }

    int remove_fd(int fd)
    {
        for (int i = 0; i < num; i++)
            if  (pfds[i].fd == fd)
            {
                pfds[i].fd = -1;
                handlers[fd] = NULL;
                return 0;
            }

        return -1;
    }

    void show_fds()
    {
        printf("poller::num: %d\n", num);
        for (int i = 0; i < num; i++)
        {
            pollfd& pfd = pfds[i];
            printf("fd: %d, events: %d, revents: %d, handler: %lx\n", 
                    pfd.fd, pfd.events, pfd.revents, (unsigned long)handlers[pfd.fd]);
        }
    }
};


class EpollHandler
{public:
    virtual int handle(uint32_t events) { return -1;}
    virtual ~EpollHandler() {}
};



class Connection : public EpollHandler
{public:
    char* buf;
    int fd;
    // int epoll_fd;
    Poller* poller;


    Connection(int fd, Poller* poller, char* buf) : fd(fd), poller(poller), buf(buf) 
    {
        poller->add_fd(fd, POLLIN, this);
    }

    virtual ~Connection() 
    {
        // if  (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) < 0)
        if  (poller->remove_fd(fd) < 0)
            perror("poller->remove_fd error");
        
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
        printf("Connection::handle: 1, events: %d\n", events);
        int ret = 0;

        if  (events & POLLIN)
        {
            if  (echo() <= 0)
                ret = -1; 
        }
        else
            ret = -1;

        printf("Connection::handle: 2, ret: %d\n", ret);
        return ret;
    }
};



void handle_accept(int listen_fd, Poller* poller, char* buf)
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

    Connection* conn = new Connection(fd, poller, buf);
}



// one loop per thread
void* loop(void* arg)
{
    int listen_fd  = *(int*)arg;
    printf("loop on fd %d\n", listen_fd);

    Poller* poller = new Poller();
    poller->add_fd(listen_fd, POLLIN, NULL);

    char* echo_buf = new char[buf_size];


    if  (listen(listen_fd, backlog) == -1)  perror("listen error");

    while (true)
    {
        int num = -1;
        if  ((num = poll(poller->pfds, poller->num, -1)) < 0)  perror("poll error");
        printf("poll num: %d\n", num);

        for (int i = 0; i < poller->num; i++)
        {   struct pollfd& pfd = poller->pfds[i];

            if  (pfd.fd == listen_fd && (pfd.revents & POLLIN))
            {
                pfd.revents = 0;
                handle_accept(listen_fd, poller, echo_buf);
            }
            else if  (pfd.revents)
            {            
                // use int handle(void* data, uint32_t events) as interface between epoll and concret handler, e.g. connection.
                struct EpollHandler* epoll_handler = (struct EpollHandler*)poller->handlers[pfd.fd];
                uint32_t evs = pfd.revents;
                pfd.revents = 0;
                printf("epoll_handler: %lx, evs: %d\n", (unsigned long)epoll_handler, evs);

                if  (epoll_handler->handle(evs) < 0)
                    delete epoll_handler;
            }
        }
    }

    close(listen_fd);
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
