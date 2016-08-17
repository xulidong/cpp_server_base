#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include<fcntl.h>

int main() {
	signal(SIGCHLD, SIG_IGN);
	printf("before fork pid:%d\n", getpid());

	pid_t pid = fork();
	if (pid == -1) {
		perror("fork error.\n");	
		return -1;
	} 
	
	if (pid > 0) {
		printf("parent pid:%d\n", getpid());
		exit(0);// 父进程退出
	} else if (pid == 0) {
		printf("child pid:%d, parent pid:%d\n", getpid(), getppid());

		/*
		 * 以下操作可以使用daemon()函数实现
		 * */

		// 为子进程创建一个新的会话(session)，之前父子进程运行在同一个会话里，父进程是会还的领头进程
		// 父进程exit后，子进程成为孤儿进程，被init进程收养
		// 执行setsid之后，子进程获得一个新的会还id，这是父进程退出之后不会影响到子进程
		pid_t id = setsid();
		if (id == -1) {
			perror("setsid");	
			exit(0);
		} else {
			printf("setsid return:%d\n", id);
		}
		// 改变守护进程的根目录
		chdir("/");	
		// 关闭默认打开的标准输入、输出、错误输出流
		int i = 0;
		for (i = 0; i < 3; i++){
			close(i);
		}
		// 把0号文件描述符指向/dev/null
		open("/dev/null", O_RDWR);
		// dup复制一个现存的文件的描述
		dup(0);//把0号文件描述符赋值给空闲的文件描述符1
		dup(1);//把0号文件描述符赋值给空闲的文件描述符2
	}

	// 加入死循环，防止进程退出
	for(;;);

	return 0;
}
