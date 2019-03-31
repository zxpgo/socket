// ChatRoom_Server.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
// 聊天室服务端实现

#include "pch.h"
#include <iostream>
#include <WinSock2.h> //socket头文件
#include <cstring>

using namespace std;
#pragma comment(lib, "ws2_32.lib") //socket库

//+++++++++++++++++++++全局变量++++++++++++++++++++
const int MAX_LINK_NUM = 10; //服务端最大连接数
const int BUFFER_SIZE = 1024; //缓冲区大小
const int port = 12345;
const int WAITE_TIME = 10; //每个客户端等待时间

int total = 0; //当前已连接的客户端数目
SOCKET cliSock[MAX_LINK_NUM]; //客户端套接字，0号为服务端，下同
WSAEVENT cliEvent[MAX_LINK_NUM]; //客户端事件
string cliName[MAX_LINK_NUM]; //客服端名称
SOCKADDR_IN cliAddr[MAX_LINK_NUM]; //客户端地址
int RECV_TIMEOUT = 10; //接收消息超时，毫秒
int SEND_TIMEOUT = 10; //发送消息超市，毫秒


DWORD WINAPI servEventThread(LPVOID lpParameter); //服务端时间处理线程

int main()
{
	//1.加载socket库============================
	WSADATA wsaData;//获取版本信息
	WSAStartup(MAKEWORD(2, 2), &wsaData); //MAKEWORD(主版本号，副版本号)

	//2.创建socket==============================
	SOCKET servSock = socket(AF_INET, SOCK_STREAM, 0); //流式套接字

	//3. 将服务器地址打开在一个结构体里面=======
	SOCKADDR_IN servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.S_un.S_addr = inet_addr("192.168.136.1"); //服务器地址设置为回传地址
	servAddr.sin_port = htons(port); //host to net shrot, 设置端口号为port

	//4. 绑定服务端的socket和打包好的地址========
	bind(servSock, (SOCKADDR*)&servAddr, sizeof(servAddr));
	
	//给服务端socket绑定一个事件对象，来接收客服端连接的事件
	WSAEVENT servEvent = WSACreateEvent(); //创建一个人工重设的未传信事件对象
	WSAEventSelect(servSock, servEvent, FD_ALL_EVENTS); //绑定事件对象并监听所有事件

	cliSock[0] = servSock;
	cliEvent[0] = servEvent;
	//5. 开启监听================================
	listen(servSock, 10); //监听队列长度为10

	//6. 创建线程
	CloseHandle(CreateThread(NULL, 0, servEventThread, (LPVOID)&servSock, 0, NULL));
	//不需要句柄所以直接关闭

	//显示服务器开启消息
	cout << " ============聊天室服务器已开启============ " << endl;
	cout << " ============限制最大连接数: " << MAX_LINK_NUM << "============" << endl;

	/*//connet test
	int addrLen = sizeof(SOCKADDR); //用于接收客服端地址包结构的长度
	SOCKET cliSock = accept(servSock, (SOCKADDR*)&servAddr, &addrLen);

	if (cliSock != INVALID_SOCKET)
	{
		cout << "连接成功！" << endl;
	}

	while (true)
	{
		char buf[10] = { 0 }; //测试用缓冲区
		int nrecv = recv(cliSock, buf, sizeof(buf), 0);
		if (nrecv > 0)
		{
			cout << buf << endl;
		}
	}
	*/

	//发送消息给全部客户端
	while (true)
	{
		char inputBuf[BUFFER_SIZE] = { 0 };
		char sendBuf[BUFFER_SIZE] = { 0 };

		cin.getline(inputBuf, sizeof(inputBuf));

		sprintf(sendBuf, "[server]:> %s", inputBuf);
		
		for (int j = 1; j <= total; j++)
		{
			
			send(cliSock[j], sendBuf, sizeof(sendBuf), 0);
		}
	}
	WSACleanup();
	return 0;

}


DWORD WINAPI servEventThread(LPVOID lpParameter) //服务端时间处理线程
{
	//这个线程中处理服务端和客服端发生的事件
	//可以理解为多个main,同时执行多个main函数
	//CreateThread(NULL, 0, servEventThread, (LPVOID)&servSock, 0, NULL)
	//上面这个语句调用了servEventThread函数，作为新的main函数

	//首先需要将传入的较少还原
	SOCKET servSock = *(SOCKET*)lpParameter;
	// lpParameter是一个void*类型，因此转换为SOCKET＊类型后再解引用，即可获得传入的SOCKET

	while (TRUE)
	{
		for (int i = 0; i < total + 1; i++)
		{
			//若有一个客服端连接，total==1, 循环2次,包含服务段和客服端

			//对每一个终端（客户端和服务端），查看是否发生事件，等待WAITE_TIME毫秒
			int index = WSAWaitForMultipleEvents(1, &cliEvent[i], false, WAITE_TIME, 0);
			index -= WSA_WAIT_EVENT_0; //此时index未发生事件的终端下标
			
			if (index == WSA_WAIT_TIMEOUT || index == WSA_WAIT_FAILED)
			{
				continue;//如果超时或出现则跳出此终端
			}
			else if (index == 0) //若此终端发生事件
			{
				WSANETWORKEVENTS networkEvent; //获取返回的结构体
				WSAEnumNetworkEvents(cliSock[i], cliEvent[i], &networkEvent);//查看是什么事件
				
				if (networkEvent.lNetworkEvents & FD_ACCEPT) //若产生accept事件（此处与位掩码相与）
				{
					if (networkEvent.iErrorCode[FD_ACCEPT_BIT] != 0) 
					{
						cout << "连接时产生错误，错误代码: " << networkEvent.iErrorCode[FD_ACCEPT_BIT] << endl;
					}
					
					//接收连接
					if (total + 1 < MAX_LINK_NUM) //若增加一个客户端仍然小于最大连接数，则接收连接请求
					{
						//total未已连接客户端数量
						int nextIndex = total + 1; //分配给新客户端的下标
						int addrLen = sizeof(SOCKADDR);
						SOCKET newSock = accept(servSock, (SOCKADDR*)&cliAddr[nextIndex], &addrLen);
						if (newSock != INVALID_SOCKET)
						{
							
							//设置发送和接收时限
							//setsockopt(newSock,SOL_SOCKET, SO_SNDTIMEO, (const char *)&SEND_TIMEOUT, sizeof(SEND_TIMEOUT));
							//setsockopt(newSock, SOL_SOCKET, SO_SNDTIMEO, (const char *)&RECV_TIMEOUT, sizeof(RECV_TIMEOUT));
							//给新客户端分配socket
							cliSock[nextIndex] = newSock;
							//接收客户端名称
							recv(cliSock[nextIndex], (char *)(&cliName[nextIndex]), BUFFER_SIZE, 0);
							

							//新客户端的地址已经存在cliAddr[nextIndex]中
							//为新客户端创建并绑定事件对象，同时设置监听close,read,write事件
							WSAEVENT newEvent = WSACreateEvent();
							WSAEventSelect(cliSock[nextIndex], newEvent, FD_CLOSE | FD_READ | FD_WRITE);
							cliEvent[nextIndex] = newEvent;
							total++; //客户端连接增加
						
							cout << "#" << nextIndex << (char *)(&cliName[nextIndex]) << "（IP：" << inet_ntoa(cliAddr[nextIndex].sin_addr)
							<<"）进入了聊天室，当前连接数：" <<  total << endl;

							//给所有用户发送欢迎消息
							char buf[BUFFER_SIZE] = "[server]:> 欢迎";
							strcat(buf, (char *)(&cliName[nextIndex]));
							strcat(buf, "(IP:");
							strcat(buf, inet_ntoa(cliAddr[nextIndex].sin_addr));
							strcat(buf, "）进入了聊天室");
							for (int j = 1; j <= total; j++)
							{
								send(cliSock[j],buf, sizeof(buf), 0);
							}
						}
					}
					
				}
				else if (networkEvent.lNetworkEvents & FD_CLOSE) //客户端被关闭即断开连接
				{
					//i表示已关闭的客户端下标
					total--;
					cout << "#" << i << (char *)(&cliName[i]) <<"（IP：" << inet_ntoa(cliAddr[i].sin_addr)
						<< "）退出了聊天室，当前连接数：" << total << endl;

					//释放这个客户端的资源
					closesocket(cliSock[i]);
					WSACloseEvent(cliEvent[i]);
					
					//数组调整，用的是顺序表删除元素的方法
					for (int j = i; j <= total; j++)
					{
						cliSock[j] = cliSock[j + 1];
						cliEvent[j] = cliEvent[j + 1];
						cliAddr[j] = cliAddr[j + 1];
						cliName[j] = cliName[j + 1];
					}
					

					//给所有客户端发送退出聊天的消息
					char buf[BUFFER_SIZE] = "[server]:>";
					strcat(buf, (char *)(&cliName[i]));
					strcat(buf,"（IP：");
					strcat(buf, inet_ntoa(cliAddr[i].sin_addr));
					strcat(buf, "）退出聊天室");
					for (int j = 1; j <= total; j++)
					{	
						send(cliSock[j], buf, sizeof(buf), 0);
					}
				}
				else if (networkEvent.lNetworkEvents & FD_READ) //接收消息
				{
					char buffer[BUFFER_SIZE] = { 0 };
					char buffer2[BUFFER_SIZE] = { 0 };
					
					for (int j = 1; j <= total; j++)
					{
						int nrecv = recv(cliSock[j], buffer, sizeof(buffer), 0);
						sprintf(buffer2, "[#%s]>: %s", (char*)&cliName[j], buffer);
					
						if (nrecv > 0) //接收到的字节大于0
						{
							//在服务端显示
							cout << buffer2 << endl;
							//给其他客户端显示
							for (int k = 1; k <= total; k++)
							{
								if (k != j)
									send(cliSock[k], buffer2, sizeof(buffer2), 0);
							}
						}

					}
					
				}
			}
		}
		
	}
	return 0;
}

