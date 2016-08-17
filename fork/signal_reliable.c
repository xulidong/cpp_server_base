#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

/*
 * 阻塞解除之后，
 * 三个可靠信号都收到了
 * 三个不可靠信号只收到了一个
 *
 * 实时信号和标准信号是根据信号值确定的，而不是发送函数
 * 由signal()安装的实时信号支持排队，不会丢失
 * 由signal()安装的标准信号支持排队，仍然会会丢失
 * 目前linux中的signal()是通过sigation()函数实现的
 * */

// 可以接收到带数据的信号
static void sigHandler(int sig) {
	printf("signal: %d\n", sig);
	// 解除阻塞
	if (sig = SIGUSR1) {
		sigset_t uset;		
		sigemptyset(&uset);
		sigaddset(&uset, SIGINT);
		sigaddset(&uset, SIGRTMIN);
		sigprocmask(SIG_UNBLOCK, &uset, NULL);
		printf("unblock...\n");
	}
}


static void actionHandler(int sig, siginfo_t* info, void* p) {
	printf("sig: %d, val: %d or  %d\n", sig, info->si_value.sival_int, info->si_int);
	if (sig = SIGUSR1) {
		sigset_t uset;		
		sigemptyset(&uset);
		sigaddset(&uset, SIGINT);
		sigaddset(&uset, SIGRTMIN);
		sigprocmask(SIG_UNBLOCK, &uset, NULL);
		printf("unblock...\n");
	}
}

int main() {
	struct sigaction act;	
	// act.sa_handler = sigHandler;
	act.sa_flags = SA_SIGINFO;// 接受额外数据需要设置这个值
	act.sa_sigaction = actionHandler;
	
	// 不可靠信号
	if(sigaction(SIGINT, &act, NULL) < 0) {
		perror("sigaction error");
		exit(EXIT_FAILURE);
	}
	
	// 可靠信号
	if(sigaction(SIGRTMIN, &act, NULL) < 0) {
		perror("sigaction error");
		exit(EXIT_FAILURE);
	}

	// 自定义信号
	if(sigaction(SIGUSR1, &act, NULL) < 0) {
		perror("sigaction error");
		exit(EXIT_FAILURE);
	}
	
	// 将信号SIGINT和SIGRTMIN添加到本进程的阻塞状态字中
	sigset_t bset;
	sigemptyset(&bset);
	sigaddset(&bset, SIGINT);
	sigaddset(&bset, SIGRTMIN);
	sigprocmask(SIG_BLOCK, &bset, NULL);

	pid_t pid = fork();
	if (pid == -1) {
		perror("fork error");
		exit(EXIT_FAILURE);
	} else if (pid == 0) {
		int i = 0;
		union sigval value;
		value.sival_int = 100;
		// 向父进程发送三次不可靠信息号
		for ( i = 0; i < 3; ++i) {
			value.sival_int += i;
			int ret = sigqueue(getppid(), SIGINT, value);	
			if (ret != 0) {
				perror("sigqueue error");
				exit(EXIT_FAILURE);
			} else {
				printf("send unreliabe signal..\n");
			}
		}

		// 向父进程发送三次可靠信息号
		for ( i = 0; i < 3; ++i) {
			value.sival_int += i;
			int ret = sigqueue(getppid(), SIGRTMIN, value);	
			if (ret != 0) {
				perror("sigqueue error");
				exit(EXIT_FAILURE);
			} else {
				printf("send reliabe signal...\n");
			}
		}

		// 向父进程发送自定义信号解除阻塞
		kill(getppid(), SIGUSR1);
	} else {
		while(1) {
			sleep(1);
		}
	}

	return 0;
}
