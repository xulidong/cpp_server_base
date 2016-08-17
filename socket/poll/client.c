#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "sockUtil.h"

/*
 * 测试服务器支持的最大链接数量
 * */

#define ERR_EXIT(msg) do {\
        fprintf(stderr, "errno:%d\n", errno);\
        perror(msg);\
        exit(EXIT_FAILURE);\
    } while(0)

int main() {
    int count = 0;
    while(1) {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            // 如果不睡眠4秒直接退出进程，服务端所有打开的套接字将关闭
            // 服务端将打印client close，影响查看输出结果
            // 因为read返回0，可能会因为某个套接字关闭而影响得真正的并发量测试
            sleep(4);
            ERR_EXIT("socket error");
        }

        struct sockaddr_in srvaddr;
        srvaddr.sin_family = AF_INET;
        srvaddr.sin_port = htons(8001);
        srvaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        if (connect(sockfd, (struct sockaddr *)&srvaddr, sizeof(struct sockaddr)) == -1) {
            ERR_EXIT("sockfdect error");
        }

        struct sockaddr_in localaddr;
        socklen_t addrlen = sizeof(localaddr);
        if (getsockname(sockfd, (struct sockaddr*)&localaddr, &addrlen) < 0) {
            ERR_EXIT("getsockname error"); 
        }
        printf("ip = %s, port = %d\n", inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port));
        printf("count = %d\n", ++count);
    }
	return 0;
}
