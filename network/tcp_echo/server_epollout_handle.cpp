#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>

#include <thread>
#include <memory>
#include <vector>
#include <map>


#include <unistd.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>



const int max_conn = 1024;
const int max_events = max_conn;
const int max_pkt_size = 1024;


class EpollHandler
{public:
    virtual int handle(uint32_t events) { return -1;}
    virtual ~EpollHandler() {}
};



struct sockaddr_in local_addr;
int thread_num = 1;
int conn_num = 1;
int pkt_size = 64;
int duration = 10;


int parse_args(int argc, char** argv)
{
    if  (argc < 2)
    {   printf("usage: %s <local port> [<threads>]\n", argv[0]);
        return -1;
    }

    int port_ = 0;
    if  (sscanf(argv[1], "%d", &port_) < 0)
    {   perror("bad port");
        return -1;
    }
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons((short)port_);
    bzero(&local_addr.sin_zero, sizeof(local_addr.sin_zero));

    if  (argc > 2)
    {   if  (sscanf(argv[2], "%d", &thread_num) < 0)
        {   perror("bad thread_num");
            return -1;
        }
    }

    printf("local_port: %d, thread_num: %d\n", port_, thread_num);
    return 0;
}



class Connection : public EpollHandler {
public:
    int fd;
    int epfd;

    Connection(int fd, int epfd)
        : fd(fd), epfd(epfd)
    {
        printf("Connection::Connection: fd: %d, epfd: %d\n", fd, epfd);

        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.ptr = this;
        if  (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) < 0)
        {   perror("epoll_ctl add error");
        }
    }

    virtual ~Connection()
    {
        if  (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL) < 0)
            perror("epoll_ctl del error");
        
        close(fd);
        printf("fd %d closed\n", fd);
    }

    virtual int handle(uint32_t ev)
    {
        printf("Connection::handle: ev: %d\n", ev);
        return 0;
    }
};

class Acceptor : public EpollHandler {
public:
    int fd;
    int epfd;

    Acceptor(struct sockaddr_in* local_addr, int epfd)
        : epfd(epfd)
    {
        fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if  (fd < 0)
        {   perror("socket error");
        }

        if  (bind(fd, (struct sockaddr*)local_addr, (socklen_t)sizeof(*local_addr)) < 0)
        {   perror("bind error");
        }

        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.ptr = this;
        if  (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) < 0)
        {   perror("epoll_ctl add error");
        }

    }

    virtual ~Acceptor()
    {
        if  (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL) < 0)
        {   perror("epoll_ctl del error");
        }

        close(fd);
    }

    int listen(int backlog)
    {
        return ::listen(fd, backlog);
    }

    virtual int handle(uint32_t ev)
    {
        printf("Acceptor::handle, ev: %d\n", ev);

        if  (ev & ~EPOLLIN)
            return -1;

        struct sockaddr_in remote_addr;
        socklen_t addr_len;
        int new_fd = ::accept4(fd, (struct sockaddr*)&remote_addr, &addr_len, SOCK_NONBLOCK);
        if  (new_fd < 0)
            return -1;

        Connection* conn = new Connection(new_fd, epfd);
        return 0;
    }
};


class EventLoop {
public:
    int epfd;
    volatile bool running;
    // std::unique_ptr<std::thread> thread;

    struct epoll_event* events;

    EventLoop()
    {
        events = new struct epoll_event[max_events];
    
        epfd = epoll_create1(0);
        if  (epfd < 0)
            perror("epoll_create1 error");


        running = true;
        // thread = std::make_unique<std::thread>( [this]() {  this->run();} );
    }

    ~EventLoop()
    {
        // thread->join();

        close(epfd);
        delete events;
    }

    void run()
    {
        while (running)
        {
            int num = epoll_wait(epfd, events, max_events, -1);
            printf("epoll_wait ret %d\n", num);

            for (int i = 0; i < num; i++)
            {
                EpollHandler* handler = (EpollHandler*)events[i].data.ptr;

                if  (handler->handle(events[i].events) < 0)
                    delete handler;
            }    
        }
    }
};


int main(int argc, char** argv)
{
    printf("hello world\n");
    if  (parse_args(argc, argv) < 0)
        return 1;

    EventLoop loop;
    // loop.thread->join();
    // sleep(10);


    Acceptor acceptor(&local_addr, loop.epfd);
    acceptor.listen(10);
    // acceptor.handle(EPOLLIN); 

    loop.run();


    return 0;
}