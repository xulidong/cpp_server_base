代码目录说明
1   endian.c        大小端测试程序
2   ip_convert.c    ip点分十进制和字符串转换
3   tcp1(echo)      使用socket实现echo程序，客户端从控制台接受输入发送给服务器，服务器原样返回客户端的数据
4   tcp2(echo)      tcp1的基础上添加添加地址复用
5   tcp3(echo)      tcp2的基础上，使用多进程，支持多客户端
6   tcp4(echo)      tcp2的基础上，改为短连接
7   tcp5(p2p)       p2p聊天程序，两个进程分别接收和发送数据
8   tcp6(p2p)       tcp5的基础上，添加进程处理，接收数据进程检测到对方关闭之后，发信号让写进程退出
9   tcp7(echo/p2p)  tcp3/tcp6的基础上，添加粘包处理:包头加长度 
10  tcp8(echo/p2p)  tcp6/tcp6的基础上，添加粘包处理:\n分界符，增加了readn,readline和writen函数
11  tcp9(echo/p2p)  tcp8/tcp8的基础上，添加僵尸进程处理和管道破裂信号处理
12  select(echo)    tcp2的基础上，使用IO复用，支持多个客户端，提取常用函数到utils/sockUtil.h 
13  api封装         在sockUtils的基础上分客户端和服务器，分别封装一套使用select实现的含timeout的api函数
14  udp1(echo)      使用udp实现echo程序，客户端从控制台接受输入发送给服务器，服务器原样返回客户端的数据
15  udpa2(数据截断) 自己给自己发送数据，多余的数据被截断
16  poll            使用poll重写select服务端，测试最大连接数 

服务器开发重难点
一 基本功

1 socket的11种状态

2 避免僵尸进程
单进程：
signal(SIGCHLD, SIG_IGN);
多进程：
static void sigHandler(int sig) {
	while(waitpid(-1, NULL, WNOHANG));
}
signal(SIGCHLD, sigHandler);

3 半链接状态发送数据导致的进程终止
signal(SIGPIPE, SIG_IGN);

4 关闭socket
close引用计数减1，为0时发送FIN字段
shutdown直接发送FIN字段

5 长链接短链接

二 套路
五种I/O模型

1 阻塞I/O模型
由Linux内核阻塞在read函数中

2 非阻塞I/O模型
Linux内核让read函数立刻返回，上层应用不断的调用read轮询

http://blog.csdn.net/Simba888888/article/category/1426325
