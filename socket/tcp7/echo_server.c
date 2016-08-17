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

static void service(int conn) {
	struct packet pack;
	memset(&pack, 0, sizeof(pack));
	int ret = 0;
	int len = 0;
	while(1) {
		// 获取包长度
		ret = readn(conn, &pack.len, sizeof(pack.len));
		if (ret == -1) {
			ERROR_EXIT();
		} else if (ret < sizeof(pack.len)) {
			printf("client exit\n");	
			break;
		}
		len = ntohl(pack.len);

		// 根据包长度来读数据
		ret = readn(conn, &pack.buf, len);
		if (ret == -1) {
			ERROR_EXIT();
		} else if (ret < len) {
			printf("client close\n");
			break;
		} else {
			// 输出，然后将数据原样返回
			fputs(pack.buf, stdout);
			writen(conn, &pack, len + sizeof(pack.len));
			memset(&pack, 0, sizeof(pack));
		}
	}
}

int main() {
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

	service(conn);

	close(conn);
	close(sockfd);

	return 0;
}
