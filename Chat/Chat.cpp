#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <stdio.h>
#include "div.h"
/*<----------------DEFINES------------------->*/
#define CREATE_USER "CREATENEWUSER"
#define UNIQUE_ID "_unique_id_"
#define MESSAGE "DEFAULT_MESSAGE"
#define TEMPLATE_SERVER_IP "127.0.0.1"
#define TEMPLATE_SERVER_PORT "8014"
#define SERVER_MSG "SERVER_MESSAGE"
#define STOP_CLIENT_WORK "#STOPCLIENT"
#define CLEAR_CONSOLE "#CLEARCONSOLE"
#define SHUTDOWN_SERVER "/SHUTDOWNSERVER"
#define HELP "/HELP"
/*<----------------DECLARATION------------------->*/
inline SOCKET sock = 0;
inline int unique_id = GetTickCount64();
constexpr int frame_rate = 1000 / 120;
inline std::string ip;
inline std::string port_s;
char msg[5096]{ '\0' };
/*<----------------PROTOTYPES------------------->*/
void write_line();
bool send_message(const std::string command);
bool create_socket(SOCKET* s, bool tcp = true);
void reg_acc();
int main(int argc, char** argv)
{
	if (!create_socket(&sock))
	{
		std::cout << "WE CANT CREATE A SOCKET. SHUTDOWN........\n";
		return -1;
	}
	std::cout << "enter the ip please >> \n";
	std::cin >> ip;
	std::cout << "enter the port please >> \n";
	std::cin >> port_s;
	/*connecting to the server*/
	struct hostent* host;
	if ((host = gethostbyname(ip.c_str())) == NULL)
	{
		std::cout << "Failed to resolve hostname.\r\n";
		WSACleanup();
		system("PAUSE");
		return 0;
	}
	SOCKADDR_IN SockAddr;
	SockAddr.sin_port = htons(std::stoi(port_s));
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);
	if (connect(sock, (SOCKADDR*)(&SockAddr), sizeof(SockAddr)) != 0)
	{
		std::cout << "Failed to establish connection with server\r\n";
		WSACleanup();
		system("PAUSE");
		return 0;
	}
	/*end of connecting part*/
	reg_acc();
	system("cls");
	char buff[512]{ '\0' };
	std::thread t(write_line);
	t.detach();
	int sz = 1;
	while (!strstr(msg, STOP_CLIENT_WORK) && sz >= 0)
	{
		send(sock, "\0", 1, 0);
		sz = recv(sock, buff, sizeof buff, 0);
		if (sz > 0)
		{
			char* start = strstr(buff, SERVER_MSG);
			if (start)
			{
				for (const char& tmp : buff)
				{
					if (&tmp == start)break;
					std::cout << tmp;
				}
				std::cout << "\n";
			}
			if (sz > 4)	ZeroMemory(&buff, 512);
		}
		if (strstr(msg, CLEAR_CONSOLE))
		{
			system("cls");
		}
		if (GetAsyncKeyState(VK_RETURN))send_message(MESSAGE);
		std::this_thread::sleep_for(std::chrono::microseconds(500));
	}
	shutdown(sock, SD_SEND);
	closesocket(sock);
	WSACleanup();
}
void write_line() {
	while (true) {
		std::cin.getline(msg, 5096);
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	}
}
bool send_message(const std::string command)
{
	if (msg[0] == ' ' || msg[0] == '\0')return false;
	static DWORD cd = GetTickCount();
	if (GetTickCount() - cd > 100)
	{
		cd = GetTickCount();
		std::string s = std::string(msg);
		s += command;
		s += UNIQUE_ID;
		s += std::to_string(unique_id);
		send(sock, s.c_str(), s.size(), 0);		
		ZeroMemory(msg, 5096);
	}

	return true;
}
bool create_socket(SOCKET* s, bool tcp)
{
	WSAData data;
	if (WSAStartup(MAKEWORD(2, 2), &data) == WSAVERNOTSUPPORTED)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	struct addrinfo* result = NULL;
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	sock = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
	if (sock == SOCKET_ERROR)
	{
		printf("creating socket failed  with error:%d", WSAGetLastError());
		WSACleanup();
		return false;
	}
	return true;
}
void reg_acc()
{
	std::string account_info_msg = "";
	std::string str_salt;
	int salt = 0;
	std::cout << "enter your name >> \n";
	std::cin >> account_info_msg;	
	bool right_salt = false;
	while (!right_salt)
	{
		right_salt = true;
		std::cout << "enter your specific key >> \n";
		std::cin >> str_salt;
		for (char tmp : str_salt)
		{
			if (tmp < '0' || tmp > '9')right_salt = false;
		}
	}
	salt = std::stoi(str_salt);
	unique_id += salt;
	account_info_msg += "_id_" + std::to_string(unique_id) + CREATE_USER;
	send(sock, account_info_msg.c_str(), account_info_msg.size(), 0);
}
