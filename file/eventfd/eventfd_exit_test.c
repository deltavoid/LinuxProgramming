#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <unistd.h>
#include <pthread.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <sys/poll.h>


int evfd;


void* poll_thread(void* arg)
{
    while (true)
    {
        struct pollfd pfd;
        pfd.fd = evfd;
        pfd.events = POLLIN;
        pfd.revents = 0;

        int ret = poll(&pfd, 1, -1);
        if  (ret < 0)  {  perror("poll error");  break;}

        printf("events: %d\n", pfd.revents);

        if  (pfd.revents & POLLIN)
        {
            uint64_t val = 0;
            if  (read(evfd, &val, sizeof(val)) != sizeof(val))
                perror("read eventfd error");
        }
    }

    return NULL;
}

void* epoll_thread(void* arg)
{
    int epfd = 0;
    if  ((epfd = epoll_create1(0)) < 0)
    {   perror("epoll_create1 error");
        return NULL;
    }

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLERR;
    event.data.fd = evfd;

    if  (epoll_ctl(epfd, EPOLL_CTL_ADD, evfd, &event) < 0)
        perror("epoll_ctl_add error");

    struct epoll_event events;

    while (true)
    {
        int num = -1;
        if  ((num = epoll_wait(epfd, &events, 1, -1)) < 0)
            perror("epoll_wait error");

        printf("events: %d\n", events.events);

        if  (events.events & EPOLLIN)
        {
            uint64_t val = 0;
            if  (read(evfd, &val, sizeof(val)) != sizeof(val))
                perror("read eventfd error");
        }
    }
}


int main()
{
    evfd = eventfd(0, EFD_NONBLOCK);

    pthread_t worker;
    {
        int ret = pthread_create(&worker, NULL, epoll_thread, NULL);
        if  (ret < 0)  perror("pthread_create error");
    }

    {
        uint64_t val = 1;
        if  (write(evfd, &val, sizeof(val)) != sizeof(val))
            perror("write eventfd error");
    }

    sleep(1);

    close(evfd);

    // {
    //     uint64_t val = 1;
    //     if  (write(evfd, &val, sizeof(val)) != sizeof(val))
    //         perror("write eventfd error");
    // }

    {
        int ret = pthread_join(worker, NULL);
        if  (ret < 0)  perror("pthread_join error");
    }
    

    return 0;
}