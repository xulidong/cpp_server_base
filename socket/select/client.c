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
		ERR_EXIT("socket error");
	}

	struct sockaddr_in srvaddr;
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(8001);
	srvaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if (connect(sockfd, (struct sockaddr *)&srvaddr, sizeof(struct sockaddr)) == -1) {
		ERR_EXIT("sockfdect error");
	}

    fd_set rset;
    FD_ZERO(&rset);

    int nready = 0;
    int fd_stdin = fileno(stdin);
    int maxfd = (fd_stdin > sockfd) ? fd_stdin : sockfd;

    char sendbuf[1024] = {0};
    char recvbuf[1024] = {0};
    while(1) {
        FD_SET(fd_stdin, &rset);
        FD_SET(sockfd, &rset);
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready == -1) {
            ERR_EXIT("select error"); 
        } else if (nready == 0) {
            continue;
        }

        if (FD_ISSET(fd_stdin, &rset)) {
            memset(sendbuf, 0, sizeof(sendbuf));
            if (fgets(sendbuf, sizeof(sendbuf), stdin) == NULL) {
                break; 
            } 
            writen(sockfd, sendbuf, strlen(sendbuf));
        }

        if (FD_ISSET(sockfd, &rset)) {
            memset(recvbuf, 0, sizeof(recvbuf));
            int ret = readline(sockfd, recvbuf, sizeof(recvbuf)); 
            if (ret == -1) {
                ERR_EXIT("readline error"); 
            } else if (ret == 0) {
                printf("client close\n");
                break;
            }

            fputs(recvbuf, stdout);
        }
    }

	close(sockfd);

	return 0;
}
