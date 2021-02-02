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


    char* buf;
    uint64_t f, p;

    Connection(int fd, int epfd)
        : fd(fd), epfd(epfd)
    {
        printf("Connection::Connection: fd: %d, epfd: %d\n", fd, epfd);

        buf = new char[max_pkt_size];
        f = p = 0;

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

        delete[] buf;
        printf("fd %d closed\n", fd);
    }




    int handle_read()
    {
        // int recv_len = ::recv(fd, buf, max_pkt_size, 0);
        // if  (recv_len <= 0)
        //     return -1;

        // int send_len = ::send(fd, buf, recv_len, 0);
        // if  (send_len != recv_len)
        //     return -1;
        if  (!(p - f <= max_pkt_size - 1))
            return 0;

        struct iovec iov[2];
        size_t iov_len = 0;

        iov[0].iov_base = buf;
        iov[0].iov_len = max_pkt_size;
        iov_len = 1;

        // if  (f % max_pkt_size <= p % max_pkt_size)
        // {   iov[0].iov_base = buf + p % max_pkt_size;
        //     iov[0].iov_len = max_pkt_size - p % max_pkt_size;
        //     if  (f % max_pkt_size >= 1)
        //     {

        //     }
        //     iov[1].iov_base = buf;
        //     iov[1].iov_len = f % max_pkt_size - 1;
        //     iov_len = 2;
        // }
        // else
        // {   iov[0].iov_base = buf + p % max_pkt_size;
        //     iov[0].iov_len = f % max_pkt_size - p % max_pkt_size - 1;
        //     iov_len = 1;
        // }

        struct msghdr msg = {
            .msg_name = NULL,
            .msg_namelen = 0,
            .msg_iov = iov,
            .msg_iovlen = iov_len,
            .msg_control = NULL,
            .msg_controllen = 0
        };
        int recv_len = recvmsg(fd, &msg, 0);
        printf("Connection::handle_read, recv_len: %d\n", recv_len);
        if  (recv_len <= 0)
            return -1;

        

        int send_len = -1;
        // if  (recv_len <= iov[0].iov_len)
        // {   iov[0].iov_len = recv_len;
        //     iov[1].iov_base = NULL;
        //     iov[1].iov_len = 0;
        //     msg.msg_iovlen = 1;
        //     send_len = sendmsg(fd, &msg, 0);
        // }
        // else
        // {   iov[1].iov_len = recv_len - iov[0].iov_len;
        //     send_len = sendmsg(fd, &msg, 0);
        // }
        iov[0].iov_len = recv_len;
        send_len = sendmsg(fd, &msg, 0);

        printf("Connection::handle_read, send_len: %d\n", send_len);

        return 0;
    }

    int handle_write()
    {
        return 0;
    }


    virtual int handle(uint32_t ev)
    {
        int ret = -1;
        printf("Connection::handle: ev: %d\n", ev);
        // return -1;

        if  (ev & ~(EPOLLIN | EPOLLOUT))
            return -1;

        // if  (ev & EPOLLOUT)
        // {
        //     ret = handle_write();
        //     if  (ret < 0)
        //         return ret;
        // }
            
        if  (ev & EPOLLIN)
        {
            ret = handle_read();
            printf("Connection::handle, handle_read: %d\n", ret);
            if  (ret < 0)
                return ret;
        }
            

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
        delete[] events;
    }

    void run()
    {
        while (running)
        {
            int num = epoll_wait(epfd, events, max_events, -1);
            printf("EventLoop::run: epoll_wait ret %d\n", num);

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