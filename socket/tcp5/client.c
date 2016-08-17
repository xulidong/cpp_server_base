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
 * p2p聊天程序客户端
 * 父进程发送数据
 * 子进程接收数据
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

	struct sockaddr_in srvaddr;
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(8001);
	srvaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if (connect(sockfd, (struct sockaddr *)&srvaddr, sizeof(struct sockaddr)) == -1) {
		ERROR_EXIT();
	}

	pid_t pid = fork();
	if (pid > 0) {
		char sendbuf[1024] = {0};
		while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) {
			int cnt = write(sockfd, sendbuf, sizeof(sendbuf));
			if (cnt == -1) {
				ERROR_EXIT();
			}
			memset(sendbuf, 0, sizeof(sendbuf));
		}
	} else if (pid == 0) {
		char recvbuf[1024] = {0};
		while(1) {
			int ret = read(sockfd, recvbuf, sizeof(recvbuf));
			if (ret == -1) {
				ERROR_EXIT();
			}
			fputs(recvbuf, stdout);
			memset(recvbuf, 0, sizeof(recvbuf));
		}
	}
	else {
		ERROR_EXIT();
	}

	close(sockfd);

	return 0;
}
