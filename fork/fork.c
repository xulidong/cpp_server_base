#include <stdio.h>
#include <signal.h>
#include <errno.h>

int main() {
	// signal 设置相应信号的处理函数
	// SIGCHLD 子进程结束信号
	// SIG_IGN 忽略信息
	signal(SIGCHLD, SIG_IGN);
	printf("before fork pid:%d\n", getpid());

	// fork创建子进程之后，规范中没有确定谁先运行，linux操作系统中父子进程执行顺序是不确定的。
	pid_t pid = fork();
	if (pid == -1) {
		perror("fork error.\n");	
		return -1;
	} 
	
	if (pid > 0) {
		printf("parent pid:%d\n", getpid());
		sleep(10);
	} else if (pid == 0) {
		printf("child pid:%d, parent pid:%d\n", getpid(), getppid());
	}
	
	printf("after fork\n");

	return 0;
}
