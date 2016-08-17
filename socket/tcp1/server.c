#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define ERROR_EXIT() {\
	printf("errno:%d\n",errno);\
	perror("error");\
	exit(EXIT_FAILURE);\
}

/*
 * 原样返回客服端发来的数据
 * */

int main() {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		ERROR_EXIT();
	}

	struct sockaddr_in srvaddr;
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(8001);
	srvaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if (bind(sockfd, (struct sockaddr *)&srvaddr, sizeof(struct sockaddr_in)) == -1) {
		ERROR_EXIT();
	}

    // listen将sockfd设为被动套接字，不能主动发报文，只能被动得等待别人链接
    // listen维护两个队列，一个是三次握手，一个是已建立链接
	if(listen(sockfd, SOMAXCONN) == -1) {
		ERROR_EXIT();
	}

	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(struct sockaddr_in); 
    // 取出已建立的链接
	int conn = accept(sockfd, (struct sockaddr*)&addr, &addrlen);
	if (conn == -1) {
		ERROR_EXIT();
	}

	printf("ip:%s : %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

	char buf[1024] = {0};
	while(1) {
		int ret = read(conn, buf, sizeof(buf));
		if (ret == -1) {
			ERROR_EXIT();
		} else if (ret == 0) {
			printf("client close\n");	
			break;
		} else {
			fputs(buf, stdout);
			int cnt = write(conn, buf, ret);
			if (cnt == -1) {
				ERROR_EXIT();
			}
		}
	}

	close(conn);
	close(sockfd);

	return 0;
}
