/*
    echo client
    one loop per thread, epoll
*/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <thread>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/timerfd.h>
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


ll get_p99(std::unordered_map<ll, ll>* tails, ll tail_num)
{
    
    std::vector<std::pair<ll, ll>> sorted;

    std::unordered_map<ll, ll>::iterator it;
    for (it = tails->begin(); it != tails->end(); it++)
        sorted.push_back(std::make_pair(it->first, it->second));

    sort(sorted.begin(), sorted.end());


    ll p99 = 0;
    ll n = 0;
    // std::map<ll, ll>::reverse_iterator rit;
    std::vector<std::pair<ll, ll>>::reverse_iterator rit;
    for (rit = sorted.rbegin(); rit != sorted.rend(); rit++)
    {
        n += rit->second;
        if  (n >= tail_num)
        {   p99 = rit->first;
            break;
        }
    }

    return p99;
}

class Total
{public:
    // ThroughputTracer::INDEX_MAX
    double data[4];

    double latencies;
    ll num;
    
    // std::map<ll, ll> tails;
    std::unordered_map<ll, ll> tails;

    Total()
    {
        memset(data, 0, sizeof(data));
        latencies = 0;
        num = 0;
    }

    ~Total() {}

    void report();

};

class ThroughputTracer
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

    ThroughputTracer()
    {
        memset(data, 0, sizeof(data));
        get_time();
    }

    ~ThroughputTracer() {}

    void get_time()  {  gettimeofday(&stamp, NULL);}

    void inc(int idx)  {  data[idx]++;}
    void add(int idx, int val)  {  data[idx] += val;}

    static void report(ThroughputTracer* now, ThroughputTracer* old, Total* total)
    {
        double dt_time = now->stamp.tv_usec - old->stamp.tv_usec 
                + (now->stamp.tv_sec - old->stamp.tv_sec) * 1000 * 1000;

        // printf("dt_time: %lf\n", dt_time);
        
        for (int i = 0; i < INDEX_MAX; i++)
        {
            double dt_val = now->data[i] - old->data[i];
            double speed = dt_val * 1000 * 1000 / dt_time;

            total->data[i] += speed;
            printf("%s: %.2lf  ", item_name[i], speed);
        }
        
        *old = *now;
    }
};

const char* ThroughputTracer::item_name[] = {
    "tx pkt/s",
    "rx pkt/s",
    "tx byte/s",
    "rx byte/s",
};

class LatencyTracer
{public:
    // this should reverse enough space
    static const int bits = 18;
    static const int size = 1 << bits;
    static const int mask = size - 1;

    ll* data;
    ll f, p;

    LatencyTracer()
    {
        data = new ll[size];
        f = p = 0;
    }

    ~LatencyTracer()
    {
        delete data;
    }

    void push(ll x)  {  data[p++ & mask] = x;}


    //call in another thread
    void report(Total* total)
    {
        double sum = 0;
        ll num = 0;
        // std::map<ll, ll> tails;

        const ll buf_bits = 8;
        const ll buf_size = 1 << buf_bits;

        while ((p >> buf_bits) - (f >> buf_bits) > 0)
        {
            for (ll i = 0; i < buf_size; i++)
            {
                ll x = data[f++ & mask];

                sum += x;
                num++;
                // tails[x]++;
                total->tails[x]++;
            }
        }

        total->num += num;
        total->latencies += sum;

        // printf("latency num: %lld\n", num);
        printf("latency avg us: %.2lf  ", sum / num);
        // printf("latency p99 us: %lld  ", ::get_p99(&tails, num / 100));
    }
};

void Total::report()
{
    for (int i = 0; i < 4; i++)
    {
        printf("%s: %.2lf  ", ThroughputTracer::item_name[i], data[i]);
    }

    // printf("latency avg us: %.2lf  ", latencies / num);
    // printf("latency p99 us: %lld\n", ::get_p99(&tails, num / 100));
    printf("\n");
}


class Connection
{public:
    int fd;
    int pkt_size;
    char* tx_buf;
    char* rx_buf;
    ThroughputTracer* tracer;
    LatencyTracer* latency_tracer;

    uint64_t tx_seq, rx_seq;
    uint64_t* err_cnt_p;

    Connection(int fd, int pkt_size, char* tx_buf, char* rx_buf, ThroughputTracer* tracer, LatencyTracer* latency_tracer, uint64_t* err_cnt_p)
        : fd(fd), pkt_size(pkt_size), tx_buf(tx_buf), rx_buf(rx_buf), tracer(tracer), latency_tracer(latency_tracer), err_cnt_p(err_cnt_p)
    {
        tx_seq = 0;
        rx_seq = 0;
    }

    ~Connection() 
    {
        close(fd);
    }

    void send()
    {
        // struct timeval* stamp = (struct timeval*)tx_buf;
        // gettimeofday(stamp, NULL);
        for (int i = 0; i < pkt_size; i += sizeof(uint64_t))
        {
            *(uint64_t*)(tx_buf + i) = tx_seq++;
        }

        int sent = ::send_full(fd, tx_buf, pkt_size, 0);
        tracer->inc(tracer->TX_PKT);
        tracer->add(tracer->TX_BYTE, sent);
    }

    int recv()
    {
        int recd = ::recv(fd, rx_buf, pkt_size, 0);
        if  (recd <= 0)
            return -1;

        if  (recd == pkt_size)
        {   
            // struct timeval now;
            // gettimeofday(&now, NULL);
            // struct timeval* old = (struct timeval*)rx_buf;
            
            // ll rtt = (now.tv_usec - old->tv_usec) 
            //         + (now.tv_sec - old->tv_sec) * 1000 * 1000;
            // // printf("rtt: %lld\n", rtt);
            // // be carefull of segment fault for array size not enough 
            // latency_tracer->push(rtt);

            for (int i = 0; i < pkt_size; i += sizeof(uint64_t))
            {
                uint64_t val = *(uint64_t*)(rx_buf + i);
                if  (val != rx_seq)
                {
                    (*err_cnt_p)++;

                    printf("fd: %d, expect val: %llu, actual val: %llu\n", 
                            fd, rx_seq, val);
                }
                
                rx_seq++;
            }
        }

        tracer->inc(tracer->RX_PKT);
        tracer->add(tracer->RX_BYTE, recd);
        // printf("fd %d recv %d bytes\n", fd, recd);

        return recd;
    }

    // void handle()
    int handle(uint32_t events)
    {
        if  (events & EPOLLIN)
        {
            if  (recv() < 0)
                return -1;

            send();

            return 0;
        }
        else
            return -1;
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

    ThroughputTracer tracer, old_tracer;
    LatencyTracer latency_tracer;
    uint64_t err_cnt;

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
            

            conns.push_back(std::make_unique<Connection>(
                    fd, pkt_size, tx_buf.data(), rx_buf.data(), &tracer, &latency_tracer, &err_cnt));

            printf("establish connection on fd %d\n", fd);
        }

        for (int i = 0; i < conns.size(); i++)
            conns[i]->send();

        running = true;
        thread = std::make_unique<std::thread>( [this] () {  this->run();} );
    }

    ~EventLoop()
    {
        printf("enter dtor\n");
        thread->join();
        close(epfd);
    }

    // work in this->thread
    void run()
    {
        while (running)
        {
            int num = -1;
            if  ((num = epoll_wait(epfd, events.data(), events.size(), -1)) < 0)  perror("epoll_wait error");
            // printf("%d events\n", num);

            for (int i = 0; i < num; i++)
            {
                int ret = conns[events[i].data.u32]->handle(events[i].events);

                if  (ret < 0)
                {   running = false;
                    break;
                }
            }
        }
    }

    // work in main thread
    int report(Total* total)
    {
        if  (!running)
            return -1;

        ThroughputTracer now(this->tracer);
        now.get_time();
        ThroughputTracer::report(&now, &this->old_tracer, total);

        // this->latency_tracer.report(total);

        printf("err_cnt: %llu", err_cnt);

        printf("\n");
        return 0;
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

int init_timerfd()
{
    int fd = timerfd_create(CLOCK_REALTIME, 0);

    struct timespec now;
    if (clock_gettime(CLOCK_REALTIME, &now) < 0)
    {   perror("clock_gettime error");
    }

    int initial_expiration = 2;
    int interval = 1;

    struct itimerspec itimerspec;
    itimerspec.it_value.tv_sec = now.tv_sec + initial_expiration;
    itimerspec.it_value.tv_nsec = now.tv_nsec;
    itimerspec.it_interval.tv_sec = interval;
    itimerspec.it_interval.tv_nsec = 0;

    if (timerfd_settime(fd, TFD_TIMER_ABSTIME, &itimerspec, NULL) < 0)
    {   perror("timerfd_settime error");
    }

    return fd;
}

int main(int argc, char** argv)
{
    if  (parse_arg(argc, argv) < 0)  return 0;

    std::vector<std::unique_ptr<EventLoop>> loops;
    for (int i = 0; i < thread_num; i++)
    {
        loops.push_back(std::make_unique<EventLoop>((struct sockaddr*)&dst_addr, conn_num, pkt_size));
    }



    int fd = init_timerfd();
    bool running = true;
    for (int i = 0; i < duration && running; i++)
    {
        uint64_t val;
        if  (read(fd, &val, sizeof(val)) != sizeof(val))
        {   perror("read timerfd error");
            break;
        }

        Total total;
        for (int j = 0; j < loops.size(); j++)
        {
            printf("%2d: ", j);
            int ret = loops[j]->report(&total);
            if  (ret < 0)
                running = false;
        }
            
        printf("all ");
        total.report();
        
        printf("\n");
    }



    for (int i = 0; i < loops.size(); i++)
        loops[i]->running = false;
    //dtor

    return 0;
}