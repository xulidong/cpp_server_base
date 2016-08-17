#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define ERR_EXIT(msg) do {\
	printf("errno:%d\n",errno);\
	perror(msg);\
	exit(EXIT_FAILURE);\
} while(0)

/*
 * 原样返回客服端发来的数据
 * udp无粘包问题
 * udp可能丢包重复
 * udp无流量控制，缓冲区满之后会覆盖
 * recvfrom返回0不代表对方关闭链接，因为udp无连接
 * udp数据发送的数据大于接收的数据，数据包将被截断，多余的数据包被丢弃
 * udp只是把数据拷贝到缓冲区，在对端没有开启的情况下仍然能发送成功
 * udp也可以调用connect，但是并没有进行三次握手，调用connect之后可以使用send函数，且不会阻塞在recvfrom
 * */

int main() {
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		ERR_EXIT("main socket error");
	}

	struct sockaddr_in srvaddr;
    memset(&srvaddr, 0, sizeof(srvaddr));
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(8002);
	srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sockfd, (struct sockaddr *)&srvaddr, sizeof(struct sockaddr_in)) == -1) {
		ERR_EXIT("main bind error");
	}

    // connect(sockfd, (struct sockaddr_in)srvaddr, sizeof(srvaddr));
        
    // 给自己发送报文，发送4个字节
    int ret = sendto(sockfd, "ABCD", 4, 0, (struct sockaddr*)&srvaddr, sizeof(srvaddr));// 调用connect之后可以调用send
    if (ret < 0) {
        ERR_EXIT("man sendto error");
    }

    int i = 0;
    // 接收4个字节，一次收一个字节
	char recvbuf[1] = {0};
    for(i = 0; i < 4; i++) {
        memset(recvbuf, 0, sizeof(recvbuf));
		ret = recvfrom(sockfd, recvbuf, sizeof(recvbuf), 0, NULL, NULL);
		if (-1 == ret) {
            if (EINTR == errno) {
                continue; 
            }
			ERR_EXIT("man recvfrom error");
        } else if (ret > 0) {
            printf("read:%d, %c\n", ret, recvbuf[0]);//read:1,A
            // 结果只收到了一次, 因为udp的数据被截断，多余的数据被丢弃
        }
	}

	close(sockfd);

	return 0;
}
