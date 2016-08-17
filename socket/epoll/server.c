#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "sockUtil.h"

/*
 * EPOLLIN 有数据到来，文件描述符可写
 * EPOLLERR 错误发生
 * EPOLLHUP 关闭
 * EPOLLLT 系统默认, 只要socket输入缓存有数据 都能够获得EPOLLIN的持续通知，
 * EPOLLET 在接收数据时， 如果有数据只会通知一次， 假如read时未读完数据，不会再有EPOLLIN的通知
 * */

#define ERR_EXIT(msg) do {\
        fprintf(stderr, "errno:%d\n", errno);\
        perror(msg);\
        exit(EXIT_FAILURE);\
    } while(0)

#define CLT_SIZE 2048

int main() {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		ERR_EXIT("main socket error");
	}

	socklen_t optval = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void*)&optval, sizeof(optval)) == -1) {
		ERR_EXIT("sockfdect error");
	}

	struct sockaddr_in srvaddr;
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(8001);
	srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sockfd, (struct sockaddr *)&srvaddr, sizeof(struct sockaddr_in)) == -1) {
		ERR_EXIT("main bind error");
	}

	if(listen(sockfd, SOMAXCONN) == -1) {
		ERR_EXIT("main listen error");
	}

    int epollfd = epoll_create(CLT_SIZE);
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = sockfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event) == -1) {
		ERR_EXIT("main epoll_ctl error");
    }

    int i = 0;
    int conn = 0;
    int nready = 0;
    struct sockaddr_in peeraddr;
    socklen_t peerlen = sizeof(peeraddr);
    struct epoll_event eventList[CLT_SIZE];
    while(1) {
        nready = epoll_wait(epollfd, eventList, CLT_SIZE, -1);
        if (nready == -1) {
            if (errno == EINTR) {
                continue;
            } 
            ERR_EXIT("poll error");
        } else if (nready == 0) {
            continue;
        }
        
        for (i = 0; i < nready; ++i) {
            struct epoll_event* pevent = &eventList[i];

            if ((pevent->events & EPOLLERR) || (pevent->events & EPOLLHUP) || !(pevent->events & EPOLLIN) ) {
                ERR_EXIT("epoll error"); 
            }

            if (pevent->data.fd == sockfd) {
                conn = accept(sockfd, (struct sockaddr*)&peeraddr, &peerlen); 
                if (conn == -1) {
                    ERR_EXIT("accept error");
                }

                if ( fcntl( sockfd, F_SETFL, fcntl( sockfd, F_GETFD, 0 ) | O_NONBLOCK ) == -1 ) {
                    ERR_EXIT("main fcntl error");
                }

                struct epoll_event event;
                event.data.fd = conn;
                event.events =  EPOLLIN | EPOLLET;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, conn, &event);
            } else {
                char recvbuf[1024] = {0}; 
                int ret = readline(conn, recvbuf, sizeof(recvbuf));
                if (ret == -1) {
                    ERR_EXIT("readline error");
                } else if (ret == 0) {
                    printf("client close\n");
                    close(conn);
                }

                fputs(recvbuf, stdout);
                writen(conn, recvbuf, strlen(recvbuf));
            }   
        }
    }

    close(sockfd);
    close(epollfd);

	return 0;
}
