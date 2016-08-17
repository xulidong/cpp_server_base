#include "../utils/sockUtil.h"

/*
 * 当服务端没有启动时，可以测试超时的情况
 * */

int main() {
    // 单进程防止半连接状态写发送信息导致进程退出
    signal(SIGPIPE, SIG_IGN);

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		ERR_EXIT("socket error");
	}

	socklen_t optval = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void*)&optval, sizeof(optval)) == -1) {
		ERR_EXIT("setsockopt error");
	}

	struct sockaddr_in srvaddr;
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(8001);
	srvaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int ret = connect_timeout(sockfd, &srvaddr, 5); 
    if (ret == -1 && errno == ETIMEDOUT) {
        printf("timeout ... \n"); 
        return 1;
    } else if (ret == -1) {
        ERR_EXIT("connect_timeout error");
    }

    struct sockaddr_in localaddr;
    socklen_t addrlen = sizeof(localaddr);
    if (getsockname(sockfd, (struct sockaddr*)&localaddr, &addrlen) == -1) {
        ERR_EXIT("getsockname error"); 
    }

    printf("ip=%s port=%d\n", inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port));

    close(sockfd);

    return 0;
}
