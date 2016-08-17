#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#include "sockUtil.h"

/*
 * linux进程默认支持的最大文件描述符数量是1024,mac默认256，
 * 可以使用ulimit来查看，getrlimit函数也可以获取，ulimit -n 或者setrlimit函数设置
 * 除去标准的0，1，2之外和监听套接字之外，还支持1020个链接
 * select可以管理的最大文件描述符数量FD_SIZE默认是1024
 * poll可以不受FD_SIZE限制，修改最大文件描述符之后可以接收更多链接 
 * poll的常用events:
 * POLLIN 有数据到来，文件描述符可写
 * POLLOUT 文件可写
 * POLLRDHUP 流式套接字关闭
 * POLLERR 错误发生
 * POLLHUP 关闭
 * */

#define ERR_EXIT(msg) do {\
        fprintf(stderr, "errno:%d\n", errno);\
        perror(msg);\
        exit(EXIT_FAILURE);\
    } while(0)

#define CLT_SIZE 2048

int main() {
    signal(SIGCHLD, SIG_IGN);

    int count = 0;

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

    // 初始化客户端连接池
    int i = 0;
    struct pollfd client[CLT_SIZE];
    for (i = 0; i < CLT_SIZE; i++) {
        client[i].fd = -1;    
    }
    client[0].fd = sockfd;
    client[0].events = POLLIN;

    int maxi = 0;// 记录已使用连接池的最大下标，避免遍历全部连接池

    // 接收新的连接
    struct sockaddr_in peeraddr;
    socklen_t peerlen = sizeof(peeraddr);
    int conn = 0;
    int nready = 0;
    while(1) {
        nready = poll(client, maxi + 1, -1);
        if (nready == -1) {
            if (errno == EINTR) {
                continue;
            } 
            ERR_EXIT("poll error");
        } else if (nready == 0) {
            continue;
        }
        
        if (client[0].revents & POLLIN) {
            // accept不再阻塞
            conn = accept(sockfd, (struct sockaddr*)&peeraddr, &peerlen); 
            if (conn == -1) {
                ERR_EXIT("accept error");
            }
            
            for (i = 0; i < CLT_SIZE; i++) {
                if (client[i].fd < 0) {
                    client[i].fd = conn; 
                    if (i > maxi) {
                        maxi = i;
                    }
                    break;
                }
            }

            // 达到最大链接则不再处理
            if (i == CLT_SIZE) {
                fprintf(stderr, "too many clients\n");
                exit(EXIT_FAILURE);
            }

            printf("recv connect count = %d ip = %s port = %d\n", ++count, inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

            client[i].events = POLLIN;
            if (--nready <= 0) {
                continue; 
            }
        }

        // 监测连接池中所有链接的输入
        for (i = 0; i <= maxi; i++) {
            conn = client[i].fd;
            if (conn == -1) {
                continue;
            }
            
            if (client[i].revents & POLLIN) {
                char recvbuf[1024] = {0}; 
                int ret = readline(conn, recvbuf, sizeof(recvbuf));
                if (ret == -1) {
                    ERR_EXIT("readline error");
                } else if (ret == 0) {
                    printf("client close\n");
                    client[i].fd = -1;
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
