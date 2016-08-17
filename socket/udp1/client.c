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


int main() {
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		ERROR_EXIT();
	}

	struct sockaddr_in srvaddr;
    memset(&srvaddr, 0, sizeof(srvaddr));
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(8002);
	srvaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int ret = 0;
	socklen_t addrlen = sizeof(struct sockaddr_in); 
	char sendbuf[1024] = {0};
	char recvbuf[1024] = {0};
	while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) {
        ret = sendto(sockfd, sendbuf, strlen(sendbuf), 0, (struct sockaddr*)&srvaddr, addrlen);
		ret = recvfrom(sockfd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*)&srvaddr, &addrlen);
		if (-1 == ret) {
            if (EINTR == errno) {
                continue; 
            }
			ERROR_EXIT();
		}
		fputs(recvbuf, stdout);
		memset(recvbuf, 0, sizeof(recvbuf));
		memset(sendbuf, 0, sizeof(sendbuf));
	}

	close(sockfd);

	return 0;
}
