// email.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
// 实现发送emial的功能


#include "pch.h"
#include <iostream>
#include <Winsock2.h> //socket头文件
#include <cstring>

#pragma comment(lib, "ws2_32.lib") //socket库
using namespace std;

const int BUF_SIZE = 2048;

/*连接服务器*/
SOCKET connect_server(char *host, int port);
/*断开连接*/
void disconnect(SOCKET c);

/*发送消息给服务器*/
void send_msg(SOCKET c, char * msg); 
/*接收服务器发送的消息*/
void recv_msg(SOCKET c);
/*Base64加密函数*/
char * base64_encode(const char * data, int data_len);

const char base[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefjhijklmnopqrstuvwxyz0123456789+/=";
int main()
{
	char sendbuf[BUF_SIZE];
	char recvbuf[BUF_SIZE];
	char recv_email[BUF_SIZE];
	char subject[BUF_SIZE];
	char content[BUF_SIZE];
	char email[200];
	char password[200];

	char * pstr = NULL;
	char host[BUF_SIZE] = "smtp.qq.com";
	int port = 25;

	//连接服务器
	SOCKET c = connect_server(host, port);

	//接收服务器发来的消息
	recv_msg(c);

	//跟服务器打招呼
	char hello_msg[BUF_SIZE] = "helo qq\r\n";
	send_msg(c, hello_msg); //\r\n表示换行回车

	//接收服务器发来的数据
	recv_msg(c);

	//发起登陆请求
	char login_msg[BUF_SIZE] = "auth login\r\n";
	send_msg(c, login_msg);

	//接收服务器发来的数据
	recv_msg(c);

	//提供用户输入邮箱地址
	cout << "========登陆邮箱========\n邮箱地址: ";
	cin >> email;
	cout << "密码：";
	cin >> password;

	/**88********发送邮件地址和密码******************/
	//加密邮箱
	pstr = base64_encode(email, strlen(email));
	
	strcpy(sendbuf, pstr);

	//添加回车换行
	strcat(sendbuf, "\r\n\0"); //\0表示字符串结尾
	send_msg(c, sendbuf);

	//接收返回内容并显示
	recv_msg(c);

	//释放内存，在base64加密函数里面分配的
	free(pstr);

	//加密邮箱密码
	pstr = base64_encode(password, strlen(password));
	strcpy(sendbuf, pstr);
	strcat(sendbuf, "\r\n\0");
	send_msg(c, sendbuf);
	recv_msg(c);
	free(pstr);

	/**************填写收件人和发件人消息***************/

	//告诉服务器发件人是谁
	strcpy(sendbuf, "mail from:<");
	strcat(sendbuf, email);
	strcat(sendbuf, ">\r\n\0");
	send_msg(c, sendbuf);
	recv_msg(c);

	//告诉服务器收件人是谁
	cout << "请输入收件人邮箱：";
	cin >> recv_email;

	strcpy(sendbuf, "rcpt to:<");
	strcat(sendbuf, recv_email);
	strcat(sendbuf, ">\r\n\0");
	send_msg(c, sendbuf);
	recv_msg(c);

	/***************发送邮件内容*****************/
	//告诉服务器，开始发送邮件内容
	char data[BUF_SIZE] = "data\r\n";
	send_msg(c,data);
	recv_msg(c);

	cout << "请输入邮件标题：";
	cin >> subject;
	cout << "请输入邮件内容：";
	cin >> content;

	//这里填写发件人，可以随便填写，可用于伪造邮件
	/*strcpy(sendbuf, "From: ");
	strcat(sendbuf, email);
	strcat(sendbuf, "\n");

	strcpy(sendbuf, "To: ");
	strcat(sendbuf, recv_email);
	strcat(sendbuf, "\n");*/

	//发送标题和内容
	//strcat(sendbuf, "subject:");
	strcpy(sendbuf, "subject:");
	strcat(sendbuf, subject);
	strcat(sendbuf, "\r\n\r\n"); //空一行
	strcat(sendbuf, content);
	strcat(sendbuf, "\r\n.\r\n\0");//输入一个.表示结束

	send_msg(c, sendbuf);
	recv_msg(c);
	
	cout << "邮件发送到：" << recv_email << endl;

	return 0;

}

//连接服务器
SOCKET connect_server(char *host, int port)
{
	
	WSADATA wsaData;
	SOCKET c;
	SOCKADDR_IN seraddr;
	struct hostent * pHostent;
	int ret = 0;
	
	//加载socket库
	ret = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (ret != 0)
		return 0;

	if (HIBYTE(wsaData.wVersion) != 2 || LOBYTE(wsaData.wVersion) != 2)
	{
		cout << "初始化失败" << endl;
		WSACleanup();
		return 1;
	}

	//创建socket
	c = socket(AF_INET, SOCK_STREAM, 0);

	pHostent = gethostbyname(host);

	//定义要连接的服务器的信息
	seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(port);
	seraddr.sin_addr.S_un.S_addr = *((unsigned long*)pHostent->h_addr_list[0]);

	//连接服务器
	connect(c, (SOCKADDR*)&seraddr, sizeof(seraddr));

	return c;
}

//断开连接，关闭socket
void disconnect(SOCKET c)
{
	closesocket(c);
	WSACleanup();
}

//接收来自服务器的消息
void recv_msg(SOCKET c)
{
	char recv_Msg[BUF_SIZE];
	//将recv_Msg初始化前BUF_SIZE个字符为'\0'
	strnset(recv_Msg, '\0', BUF_SIZE);
	//接收服务器发送的消息
	recv(c, recv_Msg, BUF_SIZE, 0);
	cout << "\nrecv: " << recv_Msg << endl;
}

//发送消息给服务器
void send_msg(SOCKET c, char * msg)
{
	cout << "\nsend: " << msg << endl;
	send(c, msg, strlen(msg), 0);
}

//base64加密
char * base64_encode(const char * data, int data_len)
{
	//int data_len = strlen(data); 
	int prepare = 0;
	int ret_len;
	int temp = 0;
	char *ret = NULL;
	char *f = NULL;
	int tmp = 0;
	char changed[4];
	int i = 0;
	ret_len = data_len / 3;
	temp = data_len % 3;
	if (temp > 0)
	{
		ret_len += 1;
	}
	ret_len = ret_len * 4 + 1;
	ret = (char *)malloc(ret_len);

	if (ret == NULL)
	{
		printf("No enough memory.\n");
		exit(0);
	}
	memset(ret, 0, ret_len);
	f = ret;
	while (tmp < data_len)
	{
		temp = 0;
		prepare = 0;
		memset(changed, '\0', 4);
		while (temp < 3)
		{
			//printf("tmp = %d\n", tmp); 
			if (tmp >= data_len)
			{
				break;
			}
			prepare = ((prepare << 8) | (data[tmp] & 0xFF));
			tmp++;
			temp++;
		}
		prepare = (prepare << ((3 - temp) * 8));
		//printf("before for : temp = %d, prepare = %d\n", temp, prepare); 
		for (i = 0; i < 4; i++)
		{
			if (temp < i)
			{
				changed[i] = 0x40;
			}
			else
			{
				changed[i] = (prepare >> ((3 - i) * 6)) & 0x3F;
			}
			*f = base[changed[i]];
			//printf("%.2X", changed[i]); 
			f++;
		}
	}
	*f = '\0';

	return ret;
}