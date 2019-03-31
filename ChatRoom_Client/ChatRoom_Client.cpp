// ChatRoom_Client.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <WinSock2.h> //socket头文件
#include <cstring>

using namespace std;
#pragma comment(lib, "ws2_32.lib") //socket库

const int NICKNAME_SIZE = 20;//名称长度
const int port = 12345; //端口
const int BUFFER_SIZE = 1024; //缓冲区大小
const int COLOR_INFO = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; //亮黄
const int COLOR_NORMAL = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN; //白色
const int COLOR_ERROR = FOREGROUND_RED | FOREGROUND_INTENSITY; //亮红色
const int COLOR_NORMAL_I = COLOR_NORMAL | FOREGROUND_INTENSITY;

char servIP[] = "192.168.136.1"; //服务器IP
char localIP[BUFFER_SIZE]; //本地IP
char client_name[NICKNAME_SIZE] = "travaler"; //用户名


DWORD WINAPI recvMsgThread(LPVOID lpParameter); //接收消息的线程
void setColor(int color); //设置文本颜色
void getLocalIP(char localIp[], int n);//获取本地IP

int main()
{
	
	//1.加载socket库============================
	WSADATA wsaData;//获取版本信息
	WSAStartup(MAKEWORD(2, 2), &wsaData); //MAKEWORD(主版本号，副版本号)

	//2.创建socket==============================
	SOCKET cliSock = socket(AF_INET, SOCK_STREAM, 0); //流式套接字,第三个参数0代表自动选择协议

	//显示客户端上线消息
	getLocalIP(localIP, sizeof(localIP) / sizeof(char));
	setColor(COLOR_INFO);
	cout << "本机IP：" << localIP << endl;
	setColor(COLOR_NORMAL);

	cout << "请输入您的昵称: ";
	cin.getline(client_name, sizeof(client_name));


	//3. 打包地址
	//3.1客服端
	SOCKADDR_IN cliAddr = { 0 };
	cliAddr.sin_addr.S_un.S_addr = inet_addr(localIP);
	cliAddr.sin_family = AF_INET;
	cliAddr.sin_port = htons(port); //端口号
	//3.2 服务端
	SOCKADDR_IN servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.S_un.S_addr = inet_addr(servIP); //服务器地址设置为回传地址
	servAddr.sin_port = htons(port); //host to net shrot, 设置端口号为port
	
			
	

	//客户端不用绑定
	if (connect(cliSock, (SOCKADDR*)&servAddr, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		setColor(COLOR_ERROR);
		cout << "连接出现错误，错误代码" << WSAGetLastError() << endl;
		
		setColor(COLOR_NORMAL);
	}
	else
	{
		send(cliSock, client_name, sizeof(client_name), 0);
	}

	//创建接收消息线程
	CloseHandle(CreateThread(NULL, 0, recvMsgThread, (LPVOID)&cliSock, NULL, 0));


	//主线程用于输入消息
	while (1) {
		char buf[100] = { 0 };
		cin.getline(buf, sizeof(buf));
		if (strcmp(buf, "quit") == 0) //若输入quit则退出聊天室
		{
			break;
		}
		send(cliSock, buf, sizeof(buf), 0);
	}

	closesocket(cliSock);
	WSACleanup();

	return 0;
}

DWORD WINAPI recvMsgThread(LPVOID lpParameter) //接收消息的线程
{
	SOCKET cliSock = *(SOCKET*)lpParameter; //获取参数：客户端socket

	while (true)
	{
		char buffer[BUFFER_SIZE] = { 0 };
		int nrecv = recv(cliSock, buffer, sizeof(buffer), 0); // nrecv是接收到的数据字节数
		if (nrecv > 0)//若接收到数据
		{
			cout << buffer << endl;
		}
		else if (nrecv < 0) //若小于说明连接断开
		{
			setColor(COLOR_INFO);
			cout << "与服务器断开连接" << endl;
			setColor(COLOR_NORMAL);
			break;
		}
	}
	return 0;
}

//获取本地IP
void getLocalIP(char localIp[], int n)
{
	gethostname(localIp, n);
	HOSTENT *host = gethostbyname(localIp);
	in_addr PcAddr;
	for (int i = 0; ; i++)
	{
		char *p = host->h_addr_list[i];
		if (NULL == p)
		{
			break;
		}
		memcpy(&(PcAddr.S_un.S_addr), p, host->h_length);
		strcpy(localIp, inet_ntoa(PcAddr));
	}
}
//设置文本颜色
void setColor(int color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}