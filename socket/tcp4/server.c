#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*
 * 原样返回客户端发来的数据
 * 添加地址复用
 * 支持多个客户端连接 
 * 改为短链接
 * */

#define ERROR_EXIT() {\
	printf("errno:%d\n",errno);\
	perror("error");\
	exit(EXIT_FAILURE);\
}


int main() {
	int i = 0;
	for(i = 0; i < 10; ++i) {
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd == -1) {
			ERROR_EXIT();
		}

		socklen_t optval = 1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void*)&optval, sizeof(optval)) == -1) {
			ERROR_EXIT();
		}

		struct sockaddr_in srvaddr;
		srvaddr.sin_family = AF_INET;
		srvaddr.sin_port = htons(8001);
		srvaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
		if (bind(sockfd, (struct sockaddr *)&srvaddr, sizeof(struct sockaddr_in)) == -1) {
			ERROR_EXIT();
		}

		if(listen(sockfd, SOMAXCONN) == -1) {
			ERROR_EXIT();
		}

		struct sockaddr_in addr;
		socklen_t addrlen = sizeof(struct sockaddr_in); 
		int conn = accept(sockfd, (struct sockaddr*)&addr, &addrlen);
		if (conn == -1) {
			ERROR_EXIT();
		}

		printf("ip:%s : %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

		char buf[1024] = {0};
		int ret = read(conn, buf, sizeof(buf));
		if (ret == -1) {
			ERROR_EXIT();
		} else if (ret == 0) {
			printf("client close\n");	
			break;
		} else {
			fputs(buf, stdout);
			memset(buf, 0, sizeof(buf));
			sprintf(buf,"i:%d\n", i);
			int cnt = write(conn, buf, ret);
			if (cnt == -1) {
				ERROR_EXIT();
			}
		}

		close(conn);
		close(sockfd);
	}
	return 0;
}
