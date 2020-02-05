/*
    echo client
    one loop per thread, epoll
*/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <memory>
#include <thread>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#include "util.h"

const int max_conn = 1024;
const int max_events = max_conn;
const int max_pkt_size = 1024;


struct sockaddr_in dst_addr;
int thread_num = 1;
int conn_num = 1;
int pkt_size = 64;
int req_num = 1000;




class Connection
{public:
    int fd;
    int req_num;
    int pkt_size;
    char* tx_buf;
    char* rx_buf;

    Connection(int fd, int pkt_size, int req_num, char* tx_buf, char* rx_buf)
        : fd(fd), pkt_size(pkt_size), req_num(req_num), tx_buf(tx_buf), rx_buf(rx_buf)
    {
    }

    ~Connection() 
    {
        close(fd);
    }

    void send()
    {
        int sent = ::send_full(fd, tx_buf, pkt_size, 0);
    }

    void recv()
    {
        int recd = ::recv(fd, rx_buf, pkt_size, 0);
        // printf("fd %d recv %d bytes\n", fd, recd);
    }

    void handle()
    {
        recv();

        send();
    }

};

class EventLoop
{public:
    
    std::vector<std::unique_ptr<Connection>> conns;
    std::vector<char> tx_buf, rx_buf;

    int epfd;
    std::vector<struct epoll_event> events;

    bool running;
    std::unique_ptr<std::thread> thread;

    EventLoop(struct sockaddr* addr, int conn_num, int pkt_size, int req_num)
        : events(conn_num), tx_buf(max_pkt_size), rx_buf(max_pkt_size)
    {
        if  ((epfd = epoll_create1(0)) < 0)  perror("epoll_create1 error");


        for (int i = 0; i  < conn_num; i++)
        {
            int fd = -1;
            if  ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)  perror("socket error");

            if  (connect(fd, addr, sizeof(struct sockaddr)) < 0)  perror("connect error");
            
            struct epoll_event event;
            event.events = EPOLLIN;
            event.data.u32 = i;
            if  (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) < 0)  perror("epoll add error");
            

            conns.push_back(std::make_unique<Connection>(fd, pkt_size, req_num, tx_buf.data(), rx_buf.data()));

            printf("establish connection on fd %d\n", fd);

        }

        running = true;
        thread = std::make_unique<std::thread>( [this] () {  this->run();} );

        for (int i = 0; i < conns.size(); i++)
            conns[i]->send();
    }

    ~EventLoop()
    {
        thread->join();
    }


    void run()
    {
        while (running)
        {
            int num = -1;
            if  ((num = epoll_wait(epfd, events.data(), events.size(), -1)) < 0)  perror("epoll_wait error");
            // printf("%d events\n", num);

            for (int i = 0; i < num; i++)
            {
                conns[events[i].data.u32]->handle();
            }
        }
    }

};

int parse_arg(int argc, char** argv)
{
    if  (argc < 3)
    {
        printf("usage: %s <dst ip> <dst port> [<threads> <conn per thread> <packet size> <req per conn>]\n", argv[0]);
        return 1;
    }

    dst_addr.sin_family = AF_INET;
    if  (inet_pton(AF_INET, argv[1], &dst_addr.sin_addr) < 0) perror("bad address");
    
    int port = 0;
    if  (sscanf(argv[2], "%d", &port) < 0)  perror("bad port");
    dst_addr.sin_port = htons((short)port);
    bzero(&dst_addr.sin_zero, sizeof(dst_addr.sin_zero));

    if  (argc > 3)
        if  (sscanf(argv[3], "%d", &thread_num) < 0)  perror("bad threads");
    
    if  (argc > 4)
        if  (sscanf(argv[4], "%d", &conn_num) < 0)  perror("bad conns");
    
    if  (argc > 5)
        if  (sscanf(argv[5], "%d", &pkt_size) < 0)  perror("bad pkt_size");

    if  (argc > 6)
        if  (sscanf(argv[6], "%d", &req_num) < 0)  perror("bad req num");

    printf("%d %d %d %d\n", thread_num, conn_num, pkt_size, req_num);
    return 0;
}

int main(int argc, char** argv)
{
    if  (parse_arg(argc, argv) < 0)  return 0;
    
    EventLoop loop((struct sockaddr*)&dst_addr, conn_num, pkt_size, req_num);



    return 0;
}