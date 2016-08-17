#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

/*
 * 1 为什么有大小端模式
 * 在C语言中，除了char只占一个直接，其他类型在cpu寄存器中都占有个字节 
 * 2 大端模式：数据高位存在地址的低字节，地位存在地址的高字节，小端模式相反
 * 3 x86小端模式，IBM大端模式，网络字节序大端模式
 * */

/*
 * 判断主机字节序的两个方式
 * */
void check1() {
	short x = 0x1122;
	char* p = (char*)&x;
	if (*p == 0x11) {
		printf("big endian\n");	
	} else if (*p = 0x22) {
		printf("little endian\n");	
	} else {
		printf("error\n");	
	}
}

void check2() {
	union U{
		short n;
		char c;
	}u;
	// big		0x00 01
	// little	0x01 00
	u.n = 1;
	if (u.c == 1) {
		printf("little endian\n");	
	} else {
		printf("big endian\n");	
	}
}

/*
 * 判断网络字节序的两个方式
 * */
void check3() {
	short x = 0x1122;
	x = htons(x);
	char* p = (char*)&x;
	if (*p == 0x11) {
		printf("big endian\n");	
	} else if (*p = 0x22) {
		printf("little endian\n");	
	} else {
		printf("error\n");	
	}
}

int main() {
	check1();	
	check2();	
	check3();	
	return 0;
}
