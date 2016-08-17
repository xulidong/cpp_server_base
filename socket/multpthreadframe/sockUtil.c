#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "sockUtil.h"

#define ERR_MSG(msg) do {\
	printf("errno:%d\n",errno);\
	perror(msg);\
} while(0)

/*
 * 成功返回成功读取的字节数，失败返回-1
 * */
ssize_t readn(int fd, void *buf, size_t count) {
    ssize_t nread = 0;
    char* pbuf = (char*)buf;
    size_t nleft = count;

    while(nleft > 0) {
        nread = read(fd, pbuf, nleft);
        if (-1 == nread) {
            if (EINTR == errno) {
                continue;
            } 
            ERR_MSG("readn read error");
            return -1;
        } else if (0 == nread) {
            printf("peer close\n");
            return count - nleft;
        } 
        pbuf += nread;
        nleft -= nread;
    } 
    return count;
}

 /*
  * 成功返回成功写入字节数，失败返回-1
  * */
ssize_t writen(int fd, void *buf, size_t count) {
    ssize_t nwrite = 0;
    char* pbuf = (char*)buf;
    size_t nleft = count;

    while(nleft > 0) {
        nwrite = write(fd, pbuf, nleft);
        if ( -1 == nwrite ) {
            if (EINTR == errno) {
                continue;
            } 
            ERR_MSG("writen read error");
            return -1;
        } else if (0 == nwrite) {
            printf("peer close\n");
            return count - nleft;
        }
        pbuf += nwrite;
        nleft -= nwrite;
    } 
    return count;
}

/*
 * 成功返回可读取的字节数，失败返回-1
 * */
ssize_t recv_peek(int fd, void *buf, size_t count) {
    while(1) {
        int ret = recv(fd, buf, count, MSG_PEEK);
        if (1 == ret&& EINTR == errno) {
            continue; 
        } else if (-1 == ret) {
            ERR_MSG("recv_peek recv error");
        }

        return ret;
    }
    return -1;
}

/*
 * 成功返回读取到的字节数，失败返回-1
 * */
size_t readline(int fd, void *buf, size_t maxCount) {
    int ret = 0;
    int nread = 0;
    int count = 0;
    int nleft = maxCount;
    char *pbuf = buf;

    while(1) {
        ret = recv_peek(fd, pbuf, nleft);
        if (-1 == ret) {
            return ret;
        } else if (0 == ret) {
            printf("peer close\n");
            return ret;
        }

        nread = ret;
        int i= 0;
        for (i = 0; i < nread; i++) {
            if ('\n' == pbuf[i]) {
                ret = readn(fd, pbuf, i + 1);
                if (ret != i + 1) {
                    return -1;
                }
                return ret + count;
            } 
        }

        if (nread > nleft) {
            return -1;
        }

        pbuf += nread;
        count += nread;
    }

    return -1;
}

/*
 * 读超时检测函数，不含读操作
 * 成功返回0，失败返回-1，超时返回-1且errno=ETIMEDOUT
 * */
int read_timeout(int fd, unsigned int wait_seconds) {
    if (0 == wait_seconds) {
        return 0; 
    } 

    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);

    struct timeval timeout;
    timeout.tv_sec = wait_seconds;
    timeout.tv_usec = 0;

    int ret = 0;
    do {
        ret  = select(fd + 1, &fdset, NULL, NULL, &timeout);
    } while(-1 == ret && EINTR == errno);
    
    if (-1 == ret) {
        ERR_MSG("read_timeout select error"); 
    } else if (0 == ret) {
        errno = ETIMEDOUT;
        ret = -1;
    }

    return 0; 
}

/*
 * 写超时检测函数，不含写操作
 * 成功返回0，失败返回-1，超时返回-1且errno=ETIMEDOUT
 * */
int write_timeout(int fd, unsigned int wait_seconds) {
    if (0 == wait_seconds) {
        return 0; 
    } 

    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);

    struct timeval timeout;
    timeout.tv_sec = wait_seconds;
    timeout.tv_usec = 0;

    int ret = 0;
    do {
        ret  = select(fd + 1, NULL, &fdset, NULL, &timeout);
    } while(-1 == ret && EINTR == errno);
    
    if (-1 == ret) {
        ERR_MSG("read_timeout select error"); 
        return -1;
    } else if (0 == ret) {
        errno = ETIMEDOUT;
        return -1;
    }

    return 0; 
}

/*
 * 成功返回已连接的套接字，失败返回-1，超时返回-1且errno=ETIMEDOUT
 * */
int accept_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds) {
    int ret = 0;
    if (wait_seconds > 0) {
        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(fd, &fdset);

        struct timeval timeout;
        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;

        do {
            ret  = select(fd + 1, &fdset, NULL, NULL, &timeout);
        } while(-1 == ret && EINTR == errno);

        if (0 == ret) {
            errno = ETIMEDOUT;
            return -1;
        } else if(-1 == ret) {
            ERR_MSG("accept_timeout select error"); 
            return -1;
        }
    }

    if (addr != NULL) {
        socklen_t addrlen = sizeof(struct sockaddr_in);
        ret = accept(fd, (struct sockaddr*)addr, &addrlen);
    } else {
        ret = accept(fd, NULL, NULL);
    }
    if (-1 == ret) {
        ERR_MSG("read_timeout accept error"); 
    }

    return ret;
}

/*
 * 设置IO为非阻塞模式
 * 成功返回0，失败返回-1
 * */
static int activate_nonblick(int fd) {
    // 获取文件描述符状态
    int flags = fcntl(fd, F_GETFL);
    if (-1 == flags) {
        ERR_MSG("activate_nonblick fcntl get error");
        return -1;
    }
    
    // 将其设置为非阻塞
    flags |= O_NONBLOCK;// 添加阻塞标记位
    flags = fcntl(fd, F_SETFL, flags);
    if (-1 == flags) {
        ERR_MSG("activate_nonblick fcntl set error");
        return -1;
    }

    return 0;
}

/*
 * 设置IO为阻塞模式
 * 成功返回0，失败返回-1
 * */
static int deactivate_nonblock(int fd) {
    // 获取文件描述符状态
    int flags = fcntl(fd, F_GETFL);
    if (-1 == flags) {
        ERR_MSG("deactivate_noblick fcntl get error");
        return -1;
    }
    
    // 将其设置为阻塞
    flags &= ~O_NONBLOCK;// 删除阻塞标记位
    flags = fcntl(fd, F_SETFL, flags);
    if (-1 == flags) {
        ERR_MSG("deactivate_noblick fcntl set error");
        return -1;
    }
    return 0;
}

/*
 * 成功返回0，失败返回-1，超时返回-1且errno=ETIMEDOUT
 * */
int connect_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds) {
    if (wait_seconds > 0) {
        activate_nonblick(fd); 
    }
    
    // 尝试链接，如果链接成功，则直接进行后续处理
    socklen_t addrlen = sizeof(struct sockaddr_in);
    // connect在阻塞情况下，默认的阻塞时间是1.5RTT大约100多秒
    int ret = connect(fd, (struct sockaddr*)addr, addrlen);
    // 如果没有链接成功，且错误码是正在建立链接，则加入到select中，设置等待时间
    if (-1 == ret && EINPROGRESS == errno) {
        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(fd, &fdset);

        struct timeval timeout;
        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;

        do {
            ret = select(fd + 1, NULL, &fdset, NULL, &timeout); 
        } while(-1 == ret && EINTR == errno);
    
        if (0 == ret) {
            errno = ETIMEDOUT;
            return -1;
        } else if (-1 == ret) {
            ERR_MSG("connect_timeout select error"); 
            return -1;
        } 
        // 可读原因可能是链接成功，或建立链接失败会写的失败信息
        int err = 0;
        socklen_t socklen = sizeof(err);
        // 通过获取错误码，如果错误码为0，则表示是链接成功
        ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &socklen);
        if (-1 == ret) {
            ERR_MSG("connect_timeout getsockopt error"); 
            return -1; 
        }
        if(0 == err) {
            ret = 0; 
        } else {
            errno  = err;
            ret = -1;
        }
    }
    
    if(wait_seconds > 0) {
        deactivate_nonblock(fd); 
    }

    return ret;
}
