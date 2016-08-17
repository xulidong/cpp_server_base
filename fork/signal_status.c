#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

static void sigHandler(int sig) {
	if (sig == SIGINT) {// "ctrl + c"发送SIGINT
		printf("recv signal: SIGINT\n");
	} else if (sig == SIGQUIT) {// "ctrl + \"发送SIGQUIT
		printf("recv signal: SIGQUIT\n", sig);
		sigset_t uset;
		sigemptyset(&uset);
		sigaddset(&uset, SIGINT);
		sigprocmask(SIG_UNBLOCK, &uset, NULL);
		signal(SIGINT, SIG_DFL);
	} else {
		printf("recv signal: %d\n", sig);
	}
}

static void print(sigset_t* pset) {
	int i = 0;
	// MSIG 64, 不明白为什么会打印65次？
	for (i = 0; i < NSIG; ++i) {
		if (i%8 == 0) {
			printf(" ");	
		}

		// 判断信号n是不是未决信号字的成员
		if (sigismember(pset, i)) {
			putchar('1');
		} else {
			putchar('0');
		}
	}
	printf("\n");
}

int main() {
	sigset_t pset;// 用来打印的信号量
	sigset_t bset;// 用来设置阻塞的信号量

	sigemptyset(&bset);
	sigaddset(&bset, SIGINT);// 将SIGINT信号放入到阻塞信号集中

	if(signal(SIGINT, sigHandler) == SIG_ERR) {
		perror("signal error");	
		exit(EXIT_FAILURE);
	}

	if(signal(SIGQUIT, sigHandler) == SIG_ERR) {
		perror("signal error");	
		exit(EXIT_FAILURE);
	}

	// 读取或更改信号屏蔽字
	// 将SIGINT(ctrl+c)设置成阻塞
	sigprocmask(SIG_BLOCK, &bset, NULL);

	while(1) {
		// 获取本进程的未决字信息
		sigpending(&pset);

		// 打印未决字
		print(&pset);	
		sleep(1);
	}

	return 0;
}
