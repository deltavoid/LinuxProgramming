#ifndef UTIL_H
#define UTIL_H
#include <sys/types.h>

typedef long long ll;

const int port = 3490;



// struct Request
// {
//     ll fd;
//     ll seq;
//     Request() { fd =0; seq = 0;}
//     Request(ll fd, ll seq) : fd(fd), seq(seq) {}
// };


extern size_t send_full(int fd, const char* msg, size_t len, int flags);
extern size_t recv_full(int fd, char* msg, size_t len, int flags);

#endif