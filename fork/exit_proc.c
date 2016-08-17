#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
/*
 * 进程终止的五种方式：
 * 正常退出：从main函数返回，库函数调用exit，系统调用_exit
 * 异常退出：调用abort产生SIGABOUT信号，ctrl+c产生终止信号SIGINT
 * */


// 输出main
int main0() {
	printf("main");		
	return 0;
}

/****************************************************************************
 * exit(0)
 * C语言库函数，调用终止处理程序，清空IO缓冲区，然后从调用_exit内核退出
 * */

void exitfun1(void) {
	printf("fun1\n");
}

void exitfun2(void) {
	printf("fun2\n");
}


// 输出main
int main3() {
	printf("main\n");	
	// 注册终止处理函数, 先注册的后执行
	atexit(exitfun1);
	atexit(exitfun2);
	exit(0);
}

/*****************************************************************************
 * _exit(0)
 * 系统调用，直接从内核退出，不会调用终止处理程序
 * */
// 没有输出
int main4() {
	printf("main");	
	atexit(exitfun1);
	atexit(exitfun2);
	_exit(0);
}

// 输出main，\n可以清空缓冲区
int main5() {
	printf("main\n");	
	atexit(exitfun1);
	atexit(exitfun2);
	_exit(0);
}

// 输出main，fflush可以清空缓冲区
int main6() {
	printf("main");	
	fflush(stdout);
	atexit(exitfun1);
	atexit(exitfun2);
	_exit(0);
}

/****************************************************************************
 * abort()
 * 异常终止，不会调用处理函数
 * */
// 输出main
int main7() {
	printf("main");	
	atexit(exitfun1);
	atexit(exitfun2);
	abort();
}

/****************************************************************************
 * ctrl+c 
 * 不会调用处理函数
 * */
// 没有输出
int main() {
	printf("main");	
	atexit(exitfun1);
	atexit(exitfun2);
	sleep(100);
	return 0;
}
