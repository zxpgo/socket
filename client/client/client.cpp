// client.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
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
char recvbuf[BUF_SIZE]; //接收信息的缓冲区


//===============================函数声明========================================



int main()
{
	//加载socket库
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 1), &wsadata) != 0) //加载成功返回0
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
	socket_Client = socket(AF_INET, SOCK_STREAM, 0);

	//初始化客户端地址包：地址和端口
	addrCli.sin_addr.s_addr = inet_addr("127.0.0.1");//127.0.0.1 相当于本地IP
	addrCli.sin_family = AF_INET; //协议族
	addrCli.sin_port = htons(12248); //端口,确保服务器和客户端端口号一致

	//初始化服务器地址包
	addrSer.sin_addr.S_un.S_addr = inet_addr("192.168.80.1");
	addrSer.sin_family = AF_INET;
	addrSer.sin_port = htons(12248);

	while (TRUE)
	{
		//连接成功
		if (connect(socket_Client, (SOCKADDR*)&addrSer, sizeof(addrSer)) != SOCKET_ERROR)
		{
			//接收信息
			recv(socket_Client, recvbuf, sizeof(sendbuf), 0);
			cout << recvbuf << endl;
		}
	}

	closesocket(socket_Server);
	closesocket(socket_Client);

	WSACleanup();

	return 0;
}