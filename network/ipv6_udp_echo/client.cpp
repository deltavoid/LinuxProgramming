#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>


const short port = 8000;
struct sockaddr_in6 server_addr;
int buf_size = 4096;
int request_num = 100;


//const char remote_ip[] ="::1";



void* client(void* arg)
{
    int fd = *(int*)arg;
    char buf[buf_size];

    struct sockaddr_in6 recv_server_addr;
    socklen_t socklen = sizeof(struct sockaddr_in6);
    const int addr_buf_len = 100;
    char addr_buf[addr_buf_len];

    for (int i = 0; i < request_num; i++)
    {
        int send_len = sendto(fd, buf, buf_size, 0, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in6));
        printf("client send %d bytes to %s:%d\n", send_len, 
                inet_ntop(AF_INET6, &server_addr.sin6_addr, addr_buf, addr_buf_len), ntohs(server_addr.sin6_port));

        int recv_len = recvfrom(fd, buf, buf_size, 0, (struct sockaddr*)&recv_server_addr, &socklen);
        printf("client recv %d bytes from %s:%d\n", recv_len, 
                inet_ntop(AF_INET6, &recv_server_addr.sin6_addr, addr_buf, addr_buf_len), ntohs(recv_server_addr.sin6_port));
        printf("\n");
        sleep(1);
    }

    close(fd);
}


int main(int argc, char** argv)
{
    if  (argc < 2)  
    {   printf("usage: ./client <host ip> [<requests> [<buf_size>]]\n");
        return 0;
    }

    // struct hostent* he;   
    // if  ((he = gethostbyname(argv[1])) == NULL)  perror("gethostbyname error");

    server_addr.sin6_family = AF_INET6;
    // server_addr.sin6_addr = *((struct in6_addr*)he->h_addr);
    inet_pton(AF_INET6, argv[1], &server_addr.sin6_addr);
    server_addr.sin6_port = htons(port);

    if  (argc >= 3)  sscanf(argv[2], "%d", &request_num);

    if  (argc >= 4)  sscanf(argv[3], "%d", &buf_size);

    
    int fd = -1;
    if  ((fd = socket(AF_INET6, SOCK_DGRAM, 0)) == -1)  perror("socket error");



    
    client(&fd);


    return 0;
}