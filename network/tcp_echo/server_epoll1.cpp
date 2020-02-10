/*
    echo server
    one loop per thread, epoll
*/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <vector>
#include <queue>
#include <set>
#include <map>
#include <memory>
#include <thread>
#include <mutex>


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

int port = 0;

class Connection
{public:
    int fd;
    int pkt_size;
    char* tx_buf;
    char* rx_buf;

    Connection(int fd, int pkt_size, char* tx_buf, char* rx_buf)
        : fd(fd), pkt_size(pkt_size), tx_buf(tx_buf), rx_buf(rx_buf)
    {
    }

    ~Connection()
    {
        close(fd);
    }

    int handle()
    {
        int recv_len = recv(fd, rx_buf, pkt_size, 0);
        // printf("recv %d bytes on fd %d\n", recv_len, fd);
        if  (recv_len <= 0)  return recv_len;

        int send_len = send(fd, rx_buf, recv_len, 0);
        return send_len;
    }

};

class EventLoop
{public:
    std::vector<std::unique_ptr<Connection>> conns;
    std::queue<std::unique_ptr<Connection>> buf_conns;
    std::mutex buf_conns_mutex;

    std::vector<char> tx_buf, rx_buf;

    int epfd;
    std::vector<struct epoll_event> events;

    bool running;
    std::unique_ptr<std::thread> thread;

    EventLoop()
        : events(max_events), tx_buf(max_pkt_size), rx_buf(max_pkt_size)
    {
        if  ((epfd = epoll_create1(0)) < 0)  perror("epoll_create1 error");

        running = true;
        thread = std::make_unique<std::thread>( [this] () {  this->run();} );
    }

    ~EventLoop()
    {
        thread->join();
        printf("EventLoop dtor\n");
    }

    void run()
    {
        while (running)
        {
            dump_conn();

            int num = -1;
            if  ((num = epoll_wait(epfd, events.data(), events.size(), 100)) < 0)  perror("epoll_wait error");
            // printf("%d events\n", num);

            for (int i = 0; i < num; i++)
            {
                int idx = events[i].data.u32;
                int ret = conns[idx]->handle();
                if  (ret < 0)  rm_conn(std::move(conns[idx]));
            }
        }
    }

    void add_conn(std::unique_ptr<Connection> conn)
    {
        // printf("add_conn\n");
        std::lock_guard<std::mutex> guard(buf_conns_mutex);
        buf_conns.push(std::move(conn));
        // buf_conns.emplace(std::move(conn));
    }

    void dump_conn()
    {
        std::lock_guard<std::mutex> guard(buf_conns_mutex);
        while (!buf_conns.empty())
        {
            std::unique_ptr<Connection> conn = std::move(buf_conns.front());
            buf_conns.pop();
            // printf("conn: %d\n", conn->fd);
            
            struct epoll_event event;
            event.events = EPOLLIN;
            event.data.u32 = conns.size();
            // printf("conns.size: %d\n", conns.size());
            if  (epoll_ctl(epfd, EPOLL_CTL_ADD, conn->fd, &event) < 0)  perror("epoll add error");

            conns.push_back(std::move(conn));
        }
    }

    void rm_conn(std::unique_ptr<Connection> conn)
    {
        if  (epoll_ctl(epfd, EPOLL_CTL_DEL, conn->fd, NULL) < 0)  perror("epoll del error");

    }

};


class Acceptor
{
public:
    int port;
    int fd;
    static const int backlog = 10;

    std::vector<std::unique_ptr<EventLoop>>* loops;

    Acceptor(int port, std::vector<std::unique_ptr<EventLoop>>* loops)
        : port(port), loops(loops)
    {
        struct sockaddr_in this_addr;
        this_addr.sin_family = AF_INET;
        this_addr.sin_port = htons((short)port);
        this_addr.sin_addr.s_addr = INADDR_ANY;
        bzero(&(this_addr.sin_zero), sizeof(this_addr.sin_zero));

        if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            perror("socket error");

        if (bind(fd, (struct sockaddr *)&this_addr, sizeof(struct sockaddr)) == -1)
            perror("bind error");
    }

    ~Acceptor()
    {
        close(fd);
    }

    void listen()
    {
        if  (::listen(fd, backlog) == -1)  perror("listen error");
    }

    // run in single thread
    void accept()
    {
        while (true)
        {
            struct sockaddr_in peer_addr;
            int size = sizeof(peer_addr);
            int conn_fd =::accept(fd, (struct sockaddr*)&peer_addr, (socklen_t*)&size);
            if  (conn_fd < 0)  perror("accept error");
            // printf("accept: %d\n", conn_fd);

            // close(conn_fd);

            int idx = conn_fd % loops->size();
            std::unique_ptr<Connection> conn = 
                    std::make_unique<Connection>(conn_fd, max_pkt_size, (*loops)[idx]->tx_buf.data(), (*loops)[idx]->rx_buf.data());
            
            (*loops)[idx]->add_conn(std::move(conn));
        }

        // for (int i = 0; i < 4; i++)
        // {
        //     printf("accept %d\n", i);
        //     std::this_thread::sleep_for(std::chrono::seconds(1));
        // }
            

    }
};


int parse_arg(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("usage: %s <port>\n", argv[0]);
        return -1;
    }

    if (sscanf(argv[1], "%d", &port) < 0)
        perror("bad port");

    return 1;
}



int main(int argc, char **argv)
{
    if (parse_arg(argc, argv) < 0)
        return 0;
    
    std::vector<std::unique_ptr<EventLoop>> loops;
    loops.push_back(std::make_unique<EventLoop>());


    Acceptor acceptor(port, &loops);
    acceptor.listen();
    acceptor.accept();
        
    for (int i = 0; i < loops.size(); i++)
        loops[i]->running = false;
    printf("hello world\n");
    return 0;
}