#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/*
 * vfork创建完进程之后必须立即执行exec或者exit
 * vfork子进程与父进程共享数据段
 * vfork子进程先执行，父进程必须在子进程执行exec或exit之后才会运行
 * */

int main() {
	int num = 10;
	pid_t pid = vfork();
	if (pid > 0) {
		num++;
		printf("parent:%d\n", num);
	} else if (pid == 0) {
		num++;
		printf("child:%d\n", num);
		exit(0);
	} else {
		perror("vfork");
	}

	return 0;
}
