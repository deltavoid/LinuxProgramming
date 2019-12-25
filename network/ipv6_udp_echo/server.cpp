#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>


const int buf_size = 4096;



void* udp_echo(void* arg)
{
    int fd = *(int*)arg;
    char buf[buf_size];

    struct sockaddr_in6 client_addr;
    socklen_t socklen = sizeof(struct sockaddr_in6);
    const int addr_buf_len = 100;
    char addr_buf[addr_buf_len];

    while (true)
    {
        int recv_len = recvfrom(fd, buf, buf_size, 0, (struct sockaddr*)&client_addr, &socklen);
        if  (recv_len <= 0)  break;
        printf("server recv %d bytes from %s:%d\n", recv_len, 
                inet_ntop(AF_INET6, &client_addr.sin6_addr, addr_buf, addr_buf_len), ntohs(client_addr.sin6_port));
        
        int send_len = sendto(fd, buf, recv_len, 0, (struct sockaddr*)&client_addr, socklen); 
        printf("server echo %d bytes to %s:%d\n", send_len, 
                inet_ntop(AF_INET6, &client_addr.sin6_addr, addr_buf, addr_buf_len), ntohs(client_addr.sin6_port));
        printf("\n");
    }

    close(fd);
    printf("socket %d closed\n", fd);
    return NULL;
}




int main(int argc, char** argv)
{
    if  (!(argc > 1))
    {   printf("usage: %s <port>\n", argv[0]);
        return 0;
    }

    struct sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(sockaddr_in6));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;

    int port_;
    if  (sscanf(argv[1], "%d", &port_) == -1)  perror("bad port");
    server_addr.sin6_port = htons((short)port_);


    int fd = -1;
    if  ((fd = socket(AF_INET6, SOCK_DGRAM, 0)) == -1)  perror("socket error");


    // addr.sin6_family=AF_INET6;
	// addr.sin6_port=htons(LOCALPORT);
	// addr.sin6_addr=in6addr_any;


    if  (bind(fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in6)) == -1)  perror("bind error");

    udp_echo(&fd);


    return 0;
}