#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

/*
 * p2p聊天程序服务端
 * 子进程发送数据
 * 父进程接收数据，检测到客户端关闭之后，发送信号让子进程关闭
 *
 * 一 粘包问题原因
 * 1 tcp是字节流无边界
 * 2 套接字发送和接受都有缓冲区
 * 3 tcp传送端有mss大小限制
 * 4 链路层有MTU大小限制，如果数据包大于这个限制要在IP层分片，导致消息分割
 * 5 tcp流量控制和拥塞控制
 * 6 tcp延迟发送机制
 * tcp/ip协议在传输层没有处理粘包问题
 *
 * 二 解决方案
 * 1 定长包
 * 2 加边界\r\n(ftp)
 * 3 包头加长度
 *
 *
 * 本例使用：包头加长度
 * */

#define ERROR_EXIT() {\
	printf("errno:%d\n",errno);\
	perror("error");\
	exit(EXIT_FAILURE);\
}

struct packet{
	int len;
	char buf[1024];
};

static ssize_t readn(int fd, void* buf, size_t count) {
	size_t nleft = count;
	ssize_t nread = 0;
	char* pbuf = (char*)buf;

	while(nleft > 0) {
		nread = read(fd, pbuf, nleft);
		if (nread < 0) {
			// 防止其他信号中断读操作
			if (errno == EINTR) {
				continue;
			}
			return -1;
		}
		else if (nread == 0) {
			break;
		}
		else {
			pbuf += nread;
			nleft -= nread;
		}
	}

	return count - nleft;
}

static ssize_t writen(int fd, const void* buf, size_t count) {
	size_t nleft = count;
	ssize_t nwrite = 0;
	char* pbuf = (char*)buf;

	while(nleft > 0) {
		nwrite = write(fd, pbuf, nleft);
		if (nwrite < 0) {
			// 防止其他信号中断写操作
			if (errno == EINTR) {
				continue;
			}
			return -1;
		} else if (nwrite == 0) {
			continue;
		} else {
			pbuf += nwrite;
			nleft -= nwrite;
		}
	}

	return count - nleft;
}

static void sigHandler(int sig) {
	if (sig == SIGUSR1) {
		printf("parent exit\n");	
		exit(EXIT_SUCCESS);
	}
}

int main() {
	signal(SIGUSR1, sigHandler);
	
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

    int len = 0;
    int ret = 0;
	struct packet recvpack;
	struct packet sendpack;
	pid_t pid = fork();
	if (pid > 0) {
        // 父进程写数据
	    memset(&sendpack, 0, sizeof(sendpack));	
	    while(fgets(sendpack.buf, sizeof(sendpack.buf), stdin) != NULL) {
            len = strlen(sendpack.buf);
            sendpack.len = htonl(len);
            writen(sockfd, &sendpack, len + sizeof(sendpack.len));
		}
	} else if (pid == 0) {
        // 子进程读数据
		while(1) {
	        memset(&recvpack, 0, sizeof(recvpack));	
            // 获取包长度
            ret = readn(sockfd, &recvpack.len, sizeof(recvpack.len));
            if (ret == -1) {
                ERROR_EXIT();
            } else if (ret < sizeof(recvpack.len)) {
                printf("server exit\n");	
                break;
            }
            len = ntohl(recvpack.len);

            // 根据包长度来读数据
            ret = readn(sockfd, recvpack.buf, len);
            if (ret == -1) {
                ERROR_EXIT();
            } else if (ret < len) {
                printf("server exit\n");	
                break;
            } else {
                fputs(recvpack.buf, stdout);
            }
		}
		close(sockfd);
		kill(getppid(), SIGUSR1);
		exit(EXIT_SUCCESS);
	}
	else {
		ERROR_EXIT();
	}

	close(sockfd);

	return 0;
}
