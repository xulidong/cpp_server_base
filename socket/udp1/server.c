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

	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(struct sockaddr_in); 
    int ret = 0;
	char recvbuf[1024] = {0};
	while(1) {
        memset(recvbuf, 0, sizeof(recvbuf));
		ret = recvfrom(sockfd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*)&addr, &addrlen);
	    printf("ip:%s : %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
		if (-1 == ret) {
            if (EINTR == errno) {
                continue; 
            }
			ERR_EXIT("man recvfrom error");
        }

        fputs(recvbuf, stdout);
        sendto(sockfd, recvbuf, ret, 0, (struct sockaddr*)&addr, addrlen);
	}

	close(sockfd);

	return 0;
}
