#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

int myDeamon(int nochdir, int noclose) {
	pid_t pid = fork();
	if (pid == -1) {
		perror("fork error.\n");	
		return -1;
	} 
	
	if (pid > 0) {
		exit(0);
	}

	pid_t id = setsid();
	if (id == -1) {
		perror("setsid");	
		exit(0);
	}

	// 改变守护进程的根目录
	if (nochdir != 0) {
		chdir("/");	
	}

	// 关闭默认打开的标准输入、输出、错误输出流
	if (noclose != 0) {
		int i = 0;
		for (i = 0; i < 3; i++){
			close(i);
		}
	}

	open("/dev/null", O_RDWR);
	dup(0);
	dup(1);

	for(;;);

	return 0;
}

int main() {
	myDeamon(1, 1);
	printf("main\n");
	return 0;
}
