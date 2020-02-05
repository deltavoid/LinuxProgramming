#ifndef UTIL_H
#define UTIL_H
#include <sys/types.h>

typedef long long ll;
typedef unsigned long long ull;


extern ssize_t send_full(int fd, const char* msg, size_t len, int flags);
extern ssize_t recv_full(int fd, char* msg, size_t len, int flags);

#endif