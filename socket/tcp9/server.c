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
 * 本例使用：加边界\n
 *
 * 半链接状态写对面写数据处理
 * 1 当服务端socket关闭之后会发送FIN字段，然后进入FIN_WAIT_1状态，收到客户端回复后进入FIN_WAIT_2状态(半链接状态)
 * 2 客户端收到FIN回复ACK字段，然后进入CLOSE_WAIT状态，若此时客户端进程被阻塞不能read，处理不了FIN，将不能进入LAST_ACK状态
 * 3 TCP是全双工，虽然服务端关闭了socket，客户端还可以继续发送数据，此时服务端将回复一个RST字段
 * 5 客户端收到RST字段后会产生一个SIGPIPE信号，SIGPIPE的默认处理方式是终止进程
 * 6 因此客户端必须处理SIGPIPE信号，否则进程将被终止
 * signal(SIGPIPE, SIG_IGN);或者signal(SIGPIPE, sigHandler);
 * 7 同理，服务端也需要做这样的处理
 *
 * */

#define ERROR_EXIT() {\
	printf("errno:%d\n",errno);\
	perror("error");\
	exit(EXIT_FAILURE);\
}

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

static ssize_t recv_peek(int conn, void* buf, size_t len) {
	while(1) {
		int ret = recv(conn, buf, len, MSG_PEEK);
		if (ret == -1 && errno == EINTR) {
			continue;	
		}
		return ret;
	}
}

static ssize_t readline(int conn, void *buf, size_t maxlen) {
	int ret = 0;
	int nread = 0;
	char* pbuf = (char*)buf;
	int nleft = maxlen;
	while(1) {
		// 拷贝缓冲区中数据，并不移出
		ret = recv_peek(conn, pbuf, nleft);
		if (ret < 0) {
			return ret;// 失败	
		} else if (ret == 0) {
			return ret;// 对方关闭
		} else {
			nread = ret;
			int i = 0;
			for (i = 0; i < nread; i++) {
				// 若是有边界则取出这段数据
				if (pbuf[i] == '\n') {
					ret = readn(conn, pbuf, i + 1);	
					if (ret != i + 1) {
						exit(EXIT_FAILURE);	
					}
					return ret;
				}
			}

			// 若读出结果大于一行的最大长度
			if (nread > nleft) {
				exit(EXIT_FAILURE);	
			}
		
			// 若缓冲区中没有\n，说明数据没有接受完，则将数据全部取出，并接收下一段收据，直到遇到\n
			nleft -= nread;
			ret = readn(conn, pbuf, nread);
			if (ret != nread) {
				exit(EXIT_FAILURE);	
			}
			pbuf += nread;
		}
	}
	return -1;
}

static void sigHandler(int sig) {
	if (sig == SIGUSR1) {
		printf("child exit\n");	
		exit(EXIT_SUCCESS);
	}
    if (sig == SIGPIPE) {
        printf("client close"); 
    }
}

int main() {
	signal(SIGUSR1, sigHandler);
    signal(SIGPIPE, sigHandler);

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
			memset(recvbuf, 0, sizeof(recvbuf));
			memset(recvbuf, 0, sizeof(recvbuf));
            int ret = readline(conn, recvbuf, sizeof(recvbuf));
            if (ret == -1) {
                ERROR_EXIT();
            } else if (ret == 0) {
                printf("server close\n");
                break;
            } else {
                fputs(recvbuf, stdout);
            }
		}
		close(conn);
		kill(pid, SIGUSR1);
		exit(EXIT_SUCCESS);
	} else if (pid == 0) {
		char sendbuf[1024] = {0};
		while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) {
		    writen(conn, sendbuf, strlen(sendbuf));
			memset(sendbuf, 0, sizeof(sendbuf));
		}
	} else {
		ERROR_EXIT();
	}

	close(sockfd);

	return 0;
}
