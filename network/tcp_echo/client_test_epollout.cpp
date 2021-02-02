#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>

#include "util.h"



const int buf_size = 1000;
char read_buf[buf_size];
char recv_buf[buf_size];

int main(int argc, char** argv)
{
    if  (argc < 3)
    {   printf("usage: %s <dst ip> <dst port>\n", argv[0]);
        return 0;
    }

    struct sockaddr_in that_addr;
    that_addr.sin_family = AF_INET;
    if  (inet_pton(AF_INET, argv[1], &that_addr.sin_addr) < 0)  perror("bad addr");

    int port_;
    if  (sscanf(argv[2], "%d", &port_) < 0)  perror("bad port");
    that_addr.sin_port = htons((short)port_);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if  (fd < 0)  perror("socket error");

    int ret = connect(fd, (struct sockaddr*)&that_addr, (socklen_t)sizeof(that_addr));
    if  (ret < 0)  perror("connect error");


    {
        uint64_t buf;
        int send_len = send(fd, &buf, sizeof(uint64_t), 0);
        if  (send_len < sizeof(uint64_t))
            return 1;
    }

    // while (true)
    {
        // scanf("%s", read_buf);
        // int len = strlen(read_buf);

        // int send_len = send_full(fd, read_buf, len, 0);
        // printf("msg %s sent.\n", read_buf);


        int64_t recv_len = recv(fd, recv_buf, buf_size, 0);
        // int64_t total = *(int64_t*)recv_buf;
        int64_t total = 1024 * 1024 * 1024;
        printf("total: %lld\n", total);

        while (recv_len < total)
        {
            int len = recv(fd, recv_buf, buf_size, 0);
            if  (len <= 0)  
            {   printf("recv error\n");
                break;
            }
            
            recv_len += len;
        }

        printf("recv done\n");
    }

    return 0;
}