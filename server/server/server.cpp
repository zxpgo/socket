// server.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <WinSock2.h> //加载Windows Socket编程头文件
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#pragma comment(lib, "ws2_32.lib") //链接ws2_32.lib库文件到此项目中
using namespace std;

//===============================全局常量=======================================
const int BUF_SIZE = 2048;


//===============================全局变量=======================================
SOCKET socket_Server, socket_Client; 
SOCKADDR_IN addrSer, addrCli; //address internet 该结构体存储IP地址和端口
int naddr = sizeof(SOCKADDR_IN);

char sendbuf[BUF_SIZE];  //发送缓冲区
char inputbuf[BUF_SIZE]; //输入内容的缓冲区
char recebuf[BUF_SIZE]; //接收信息的缓冲区


//===============================函数声明========================================



int main()
{
//加载socket库
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2,2), &wsadata ) != 0 ) //加载成功返回0
	//MAKEWORD将2、2转换为一个WORD类型，其实是一个宏；WORD类型：无符号的短整型
	{
		//输出加载Socket库错误
		cout << "载入socket库失败！" << endl;
		system("pause"); 
		//让操作系统来暂停该程序进程的执行，同时程序运行到此语句处时，会在程序的窗口上显示“Press any key to continue . . .” 
		//也就是 “按任意键继续...”，即等待用户按下任意一个按键使该程序从暂停状态返回到执行状态继续从刚才暂停的地方开始执行。
		return 0;
	}

//创建socket
	socket_Server = socket(AF_INET, SOCK_STREAM, 0);

//初始化地址包：地址和端口
	addrSer.sin_addr.s_addr = inet_addr("192.168.80.1");//IP地址
	addrSer.sin_family = AF_INET; //协议族
	addrSer.sin_port = htons(12248); //端口,确保服务器和客户端端口号一致

//绑定Socket和本地地址
	bind(socket_Server, (SOCKADDR*)&addrSer, sizeof(SOCKADDR));  //SOCKADDR类型把地址和端口放在同一个变量中

//监听连接请求
	while (TRUE)
	{
		//监听连接请求
		listen(socket_Server, 5);

		//接收连接的请求，服务器创建一个socket副本用来处理接收到的请求
		socket_Client = accept(socket_Server, (SOCKADDR*)&addrCli, &naddr); //指针类型参数,接收accept的返回值

		if (socket_Client != INVALID_SOCKET) //连接成功
		{
			cout << "连接成功！" << endl;
			//发送连接成功的信息
			strcpy(sendbuf, "hello!"); //设置缓冲区内容
			send(socket_Client, sendbuf, sizeof(sendbuf), 0);
		}
	}

	closesocket(socket_Client);
	closesocket(socket_Server);

	WSACleanup();

	return 0;
}


