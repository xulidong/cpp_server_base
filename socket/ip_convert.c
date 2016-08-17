#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*
 * ipv4在网络传输用了4字节表示的点分十进制
 * */

int main() {
    // 使用字符串转创建一个32位点分十进制ip
    in_addr_t addr = inet_addr("127.0.0.1");
    printf("%u\n", addr);

    // 字符串转32位点分十进制
    struct in_addr netAddr;
    if (inet_aton("127.0.0.1", &netAddr) == 0) { // aton: address to net
        perror("inet_aton error");
        exit(EXIT_FAILURE);
    }
    printf("%u\n", netAddr.s_addr);

    // 点分十进制转字符串
    char* strAddr = inet_ntoa(netAddr);// ntoa: net to address
    printf("%s\n", strAddr);

    return 0;
}
