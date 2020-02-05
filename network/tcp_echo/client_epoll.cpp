/*
    echo client
    one loop per thread, epoll
*/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <vector>
#include <memory>
#include <thread>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/time.h>
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
int duration = 10;



class Tracer
{public:
    enum Item
    {   TX_PKT = 0,
        RX_PKT,
        TX_BYTE,
        RX_BYTE,
        INDEX_MAX,
    };
    static const char* item_name[];
    
    unsigned long long data[INDEX_MAX];
    struct timeval stamp;

    Tracer()
    {
        memset(data, 0, sizeof(data));
        get_time();
    }


    ~Tracer() {}

    void get_time()  {  gettimeofday(&stamp, NULL);}

    void inc(int idx)  {  data[idx]++;}
    void add(int idx, int val)  {  data[idx] += val;}


    static void report(Tracer* now, Tracer* old)
    {
        double dt_time = now->stamp.tv_usec - old->stamp.tv_usec 
                + (now->stamp.tv_sec - old->stamp.tv_sec) * 1000 * 1000;

        // printf("dt_time: %lf\n", dt_time);
        
        for (int i = 0; i < INDEX_MAX; i++)
        {
            double dt_val = now->data[i] - old->data[i];
            double speed = dt_val * 1000 * 1000 / dt_time;
            printf("%s: %.2lf\n", item_name[i], speed);
        }
        printf("\n");

        *old = *now;
    }
};

const char* Tracer::item_name[] = {
    "tx pkt/s",
    "rx pkt/s",
    "tx byte/s",
    "rx byte/s",
};


class Connection
{public:
    int fd;
    int pkt_size;
    char* tx_buf;
    char* rx_buf;
    Tracer* tracer;

    Connection(int fd, int pkt_size, char* tx_buf, char* rx_buf, Tracer* tracer)
        : fd(fd), pkt_size(pkt_size), tx_buf(tx_buf), rx_buf(rx_buf), tracer(tracer)
    {
    }

    ~Connection() 
    {
        close(fd);
    }

    void send()
    {
        int sent = ::send_full(fd, tx_buf, pkt_size, 0);
        tracer->inc(tracer->TX_PKT);
        tracer->add(tracer->TX_BYTE, sent);
    }

    void recv()
    {
        int recd = ::recv(fd, rx_buf, pkt_size, 0);
        tracer->inc(tracer->RX_PKT);
        tracer->add(tracer->RX_BYTE, recd);
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

    Tracer tracer;

    EventLoop(struct sockaddr* addr, int conn_num, int pkt_size)
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
            

            conns.push_back(std::make_unique<Connection>(fd, pkt_size, tx_buf.data(), rx_buf.data(), &tracer));

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
        close(epfd);
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
        printf("usage: %s <dst ip> <dst port> [<threads> <conn per thread> <packet size> <duration>]\n", argv[0]);
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
        if  (sscanf(argv[6], "%d", &duration) < 0)  perror("bad duration");

    printf("%d %d %d %d\n", thread_num, conn_num, pkt_size, duration);
    return 0;
}

int main(int argc, char** argv)
{
    if  (parse_arg(argc, argv) < 0)  return 0;
    
    EventLoop loop((struct sockaddr*)&dst_addr, conn_num, pkt_size);

    Tracer old;
    for (int i = 0; i < duration; i++)
    {
        sleep(1);
        
        Tracer now(loop.tracer);
        now.get_time();
        
        Tracer::report(&now, &old);
    }

    loop.running = false;
    // loop dtor

    return 0;
}