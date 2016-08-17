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
 * 半链接状态写对面写数据处理
 * 1 当服务端socket关闭之后会发送FIN字段，然后进入FIN_WAIT_1状态，收到客户端回复后进入FIN_WAIT_2状态(半链接状态)
 * 2 客户端收到FIN回复ACK字段，然后进入CLOSE_WAIT状态，若此时客户端进程被阻塞不能read，处理不了FIN，将不能进入LAST_ACK状态
 * 3 TCP是全双工，虽然服务端关闭了socket，客户端还可以继续发送数据，此时服务端将回复一个RST字段
 * 5 客户端收到RST字段后会产生一个SIGPIPE信号，SIGPIPE的默认处理方式是终止进程
 * 6 因此客户端必须处理SIGPIPE信号，否则进程将被终止
 * signal(SIGPIPE, SIG_IGN);或者signal(SIGPIPE, sigHandler);
 * 7 同理，服务端也需要做这样的处理
 */

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

	struct sockaddr_in srvaddr;
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(8001);
	srvaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if (connect(sockfd, (struct sockaddr *)&srvaddr, sizeof(struct sockaddr)) == -1) {
		ERROR_EXIT();
	}

	char sendbuf[1024] = {0};
	char recvbuf[1024] = {0};
	while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) {
		int cnt = write(sockfd, sendbuf, sizeof(sendbuf));
		if (cnt == -1) {
			ERROR_EXIT();
		}
		int ret = read(sockfd, recvbuf, sizeof(recvbuf));
		if (ret == -1) {
			ERROR_EXIT();
		}
		fputs(recvbuf, stdout);
		memset(recvbuf, 0, sizeof(recvbuf));
		memset(sendbuf, 0, sizeof(sendbuf));
	}

	close(sockfd);

	return 0;
}
