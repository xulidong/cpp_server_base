#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "sockUtil.h"

/*
 * 原样返回客户端发来的数据
 * 添加地址复用
 * 使用select支持多个客户端连接 
 * */

#define ERR_EXIT(msg) do {\
        fprintf(stderr, "errno:%d\n", errno);\
        perror(msg);\
        exit(EXIT_FAILURE);\
    } while(0)

int main() {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		ERR_EXIT("main socket error");
	}

	// 地址复用, 使服务器不必等待TIME_WAIT状态消失就可以重启服务器
	socklen_t optval = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void*)&optval, sizeof(optval)) == -1) {
		ERR_EXIT("sockfdect error");
	}

	struct sockaddr_in srvaddr;
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(8001);
	srvaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if (bind(sockfd, (struct sockaddr *)&srvaddr, sizeof(struct sockaddr_in)) == -1) {
		ERR_EXIT("main bind error");
	}

	if(listen(sockfd, SOMAXCONN) == -1) {
		ERR_EXIT("main listen error");
	}


    // 初始化客户端连接池
    int i = 0;
    int client[FD_SETSIZE];
    for (i = 0; i < FD_SETSIZE; i++) {
        client[i] = -1;    
    }

    int maxi = 0;// 记录已使用连接池的最大下标，避免遍历全部连接池
    int maxfd = sockfd;
    
    fd_set rset;
    fd_set allset;
    FD_ZERO(&rset);
    FD_ZERO(&allset);
    FD_SET(sockfd, &allset);

    // 接收新的连接
    struct sockaddr_in peeraddr;
    socklen_t peerlen = sizeof(peeraddr);
    int conn;
    int nready = 0;
    while(1) {
        // 监测rset中所有fd的状态
        rset = allset; 
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready == -1) {
            if (errno == EINTR) {
                continue;
            } 
            ERR_EXIT("select error");
        } else if (nready == 0) {
            continue;
        }
        
        // 若新连接到来，则将其加入到连接池中
        if (FD_ISSET(sockfd, &rset)) {
            conn = accept(sockfd, (struct sockaddr*)&peeraddr, &peerlen); 
            if (conn == -1) {
                ERR_EXIT("accept error");
            }
            
            for (i = 0; i < FD_SETSIZE; i++) {
                if (client[i] < 0) {
                    client[i] = conn; 
                    if (i > maxi) {
                        maxi = i;
                    }
                    break;
                }
            }

            // 达到最大链接则不再处理
            if (i == FD_SETSIZE) {
                fprintf(stderr, "too many clients\n");
                exit(EXIT_FAILURE);
            }

            printf("recv connect ip = %s port = %d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

            FD_SET(conn, &allset);
            if (conn > maxfd) {
                maxfd = conn; 
            }

            if (--nready <= 0) {
                continue; 
            }
        }

        // 监测连接池中所有链接的输入
        for (i = 0; i <= maxi; i++) {
            conn = client[i];
            if (conn == -1) {
                continue;
            }
            
            if (FD_ISSET(conn, &rset)) {
                char recvbuf[1024] = {0}; 
                int ret = readline(conn, recvbuf, sizeof(recvbuf));
                if (ret == -1) {
                    ERR_EXIT("readline error");
                } else if (ret == 0) {
                    printf("client close\n");
                    FD_CLR(conn, &allset);
                    client[i] = -1;
                    close(conn);
                }

                fputs(recvbuf, stdout);
                writen(conn, recvbuf, strlen(recvbuf));
                if (--nready <= 0) {
                    break;
                }
            }
        }
    }

	return 0;
}
