#ifndef __COMM_SOCKET_H__
#define __COMM_SOCKET_H__

#ifdef __cplusplus
extern 'C' {
#endif

#include <stdlib.h>
#include <netinet/in.h>

ssize_t readn(int fd, void *buf, size_t count);

ssize_t writen(int fd, void *buf, size_t count);

size_t readline(int fd, void *buf, size_t maxCount);

int read_timeout(int fd, unsigned int wait_seconds);

int write_timeout(int fd, unsigned int wait_seconds);

int accept_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds);

int connect_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds); 

#ifdef __cplusplus
}
#endif

#endif
