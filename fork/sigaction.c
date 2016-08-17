#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

/*
 * int sigaction(int signum, const struct sigaction* act, const struct sigaction* old);
 * signum除了SIGINIT和SIGQUIT之外的全部信息
 * act
 * old 返回旧的信号处理函数，可以NULL
 *
 * int sigqueue(pid_t pid, int sig, const union sigval value);类似kill，可以在发送信号的时候传递数据
 *
 * */

static void sigHandler(int sig) {
	printf("recv signal:%d\n", sig);
	exit(0);
}

static void sigAction(int sig, siginfo_t* info, void *p) {
	printf("recv signal:%d\n", sig);
	// 数据同时放在了这两个地方
	printf("%d, %d\n", info->si_value.sival_int, info->si_int);
}

int main() {
	struct sigaction act;
	// sa_handler和sa_sigaction不能同时存在, 如果父进程要接受带有数据的信号,sa_flags值必须为SA_SIGINFO
	// act.sa_handler = sigHandler;
	act.sa_sigaction = sigAction;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;

	if (sigaction(SIGINT, &act, NULL) < 0) {
		perror("sigaction error");
		return 0;		
	}

	pid_t pid = fork();
	if (pid == -1) {
		perror("sigaction error");
		return 0;		
	} else if (pid == 0) {
		union sigval val;
		val.sival_int = 123;
		int ret = sigqueue(getppid(), SIGINT, val);
		if (ret != 0) {
			exit(0);
		} else {
			sleep(2);
		}
	} else {
		while(1){}	
	}

	return 0;
}
