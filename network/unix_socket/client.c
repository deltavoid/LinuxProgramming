#include <stdio.h>  
#include <stdlib.h>  
#include <stddef.h>  
#include <string.h> 
#include <errno.h>  

#include <unistd.h>  
#include <sys/socket.h>  
#include <sys/un.h>  

 

 
#define MAXLINE 80  
 
char *client_path = "client.socket";  

char *default_server_path = "server.socket";  
char* server_path = NULL;
 
int main(int argc, char** argv) 
{  
    struct  sockaddr_un cliun, serun;  
    int len;  
    char buf[100];  
    int sockfd, n;  
 

    server_path = default_server_path;
    if  (argc > 1)
    {   server_path = argv[1];
    }




    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){  
        perror("client socket error");  
        exit(1);  
    }  
      
    // // 一般显式调用bind函数，以便服务器区分不同客户端  
    // memset(&cliun, 0, sizeof(cliun));  
    // cliun.sun_family = AF_UNIX;  
    // strcpy(cliun.sun_path, client_path);  
    // len = offsetof(struct sockaddr_un, sun_path) + strlen(cliun.sun_path);  
    // unlink(cliun.sun_path);  
    // if (bind(sockfd, (struct sockaddr *)&cliun, len) < 0) {  
    //     perror("bind error");  
    //     exit(1);  
    // }  
 
    memset(&serun, 0, sizeof(serun));  
    serun.sun_family = AF_UNIX;  
    strcpy(serun.sun_path, server_path);  
    len = offsetof(struct sockaddr_un, sun_path) + strlen(serun.sun_path);  
    if (connect(sockfd, (struct sockaddr *)&serun, len) < 0){  
        perror("connect error");  
        exit(1);  
    }  
 
    while(fgets(buf, MAXLINE, stdin) != NULL) {    
         write(sockfd, buf, strlen(buf));    
         n = read(sockfd, buf, MAXLINE);    
         if ( n < 0 ) {    
            printf("the other side has been closed.\n");    
         }else {    
            write(STDOUT_FILENO, buf, n);    
         }    
    }   
    close(sockfd);  
    return 0;  
}