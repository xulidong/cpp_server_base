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
 * 原样返回客户端发来的数据
 * 添加地址复用
 * 支持多个客户端连接 
 *
 * 添加僵尸进程处理
 * 子进程退出之后，父进程没有等待子进程的退出，会产生僵尸进程
 * 简单的处理方式是忽略子进程退出信号signal(SIGCHLD, SIG_IGN)，让1号进程处理
 * 或者使用wait(NULL);等待第一个进程的退出，避免产生僵尸进程
 * 但是当有多个链接同时断开时，会有多个子进程同时断开，产生多个SIGCHLD信号，
 * 由于SIGCHLD是不稳定信号，父进程可能接受不完全，从而产生僵尸进程
 * 
 * 半链接状态写对面写数据处理
 * 1 当服务端socket关闭之后会发送FIN字段，然后进入FIN_WAIT_1状态，收到客户端回复后进入FIN_WAIT_2状态(半链接状态)
 * 2 客户端收到FIN回复ACK字段，然后进入CLOSE_WAIT状态，若此时客户端进程被阻塞不能read，处理不了FIN，将不能进入LAST_ACK状态
 * 3 TCP是全双工，虽然服务端关闭了socket，客户端还可以继续发送数据，此时服务端将回复一个RST字段
 * 5 客户端收到RST字段后会产生一个SIGPIPE信号，SIGPIPE的默认处理方式是终止进程
 * 6 因此客户端必须处理SIGPIPE信号，否则进程将被终止
 * signal(SIGPIPE, SIG_IGN);或者signal(SIGPIPE, sigHandler);
 * 7 同理，服务端也需要做这样的处理
 * */

static void sigHandler(int sig) {
	// -1 同组进程中所有子进程
	// WNOHANG 若pid指定的子进程没有结束，则waitpid()函数返回0，不予以等待。若结束，则返回该子进程的ID。
	while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
	signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, sigHandler);

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


	// 支持多并发
	int conn = 0;
	while(1) {
		struct sockaddr_in addr;
		socklen_t addrlen = sizeof(struct sockaddr_in); 
		conn = accept(sockfd, (struct sockaddr*)&addr, &addrlen);
		if (conn == -1) {
			ERROR_EXIT();
		}

		printf("ip:%s : %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

		pid_t pid = fork();
		if (pid == 0) {
			close(sockfd);// 子进程不需要监听
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
		} else if (pid > 0) {
			close(conn);// 父进程不需要交互
		} else {
			close(conn);
			close(sockfd);
			ERROR_EXIT();
		}
	}

	close(conn);
	close(sockfd);

	return 0;
}
