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
 * p2p聊天程序服务端
 * 父进程接收数据
 * 子进程发送数据
 * */

#define ERROR_EXIT() {\
	printf("errno:%d\n",errno);\
	perror("error");\
	exit(EXIT_FAILURE);\
}


int main() {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		ERROR_EXIT();
	}

	// 地址复用, 使服务器不必等待TIME_WAIT状态消失就可以重启服务器
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

	pid_t pid = fork();
	if (pid > 0) {
		char recvbuf[1024] = {0};
		while(1) {
			int ret = read(conn, recvbuf, sizeof(recvbuf));
			if (ret == -1) {
				ERROR_EXIT();
			} else if (ret == 0) {
				printf("client close\n");	
				exit(EXIT_SUCCESS);
			} else {
				fputs(recvbuf, stdout);
				memset(recvbuf, 0, sizeof(recvbuf));
			}
		}
	} else if (pid == 0) {
		char sendbuf[1024] = {0};
		while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) {
			int cnt = write(conn, sendbuf, sizeof(sendbuf));
			if (cnt == -1) {
				ERROR_EXIT();
			}
			memset(sendbuf, 0, sizeof(sendbuf));
		}
	} else {
		ERROR_EXIT();
	}

	close(conn);
	close(sockfd);

	return 0;
}
