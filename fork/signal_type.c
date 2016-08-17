#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

/*
 * 信号的粗粒方式有三种：
 * 1 执行默认操作
 * 2 捕获并处理
 * 3 忽略信号(有两个信号不能忽略：SIGKILL/SIGSTOP)
 * 信号的特殊处理方式：
 * SIG_IGN 父进程忽略子进程的退出,例如避免僵尸进程的方式之一signal(SIGCHLD, SIG_IGN);
 * SIG_DFL 信号的默认处理方式
 *
 * 可靠信号都是实时信号，实时信号支持排队
 * */

void handler(int sig) {
	printf("recv signal:%d\n", sig);
}

int main() {
	//  注册一个信号，ctrl+c产生SIGINT信号, 终端应用程序
	signal(SIGINT, handler);	
	char c;
	printf("a to exit...");
	while((c = getchar()) != 'a') {
		pause();
	} 
	
	// 让信号回复默认行为
	signal(SIGINT, SIG_DFL);


	return 0;
}
