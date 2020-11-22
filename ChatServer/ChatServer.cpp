#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")
#include <iostream>
#include <stdio.h>
#include <string>
#include <optional>
#include <chrono>
#include <thread>
#include <functional>
#include "div.h"
/*<----------------DEFINES------------------->*/
#define DEFAULT_BUFLEN 512
#define SERVER_IP "192.168.0.105"
#define SERVER_PORT "8014"
#define CREATE_USER "CREATENEWUSER"
#define MESSAGE "DEFAULT_MESSAGE"
#define SERVER_MSG "SERVER_MESSAGE"
/*<----------------DECLARATION------------------->*/
struct user {
	std::string name;
	SOCKET attached{ 0 };
	user(std::string name)
	{
		this->name = name;
		attached = 0;
	}
	user(std::string name, SOCKET id)
	{
		this->name = name;		
		attached = id;
	}
	user() {
		this->name = "";
		attached = 0;
	};
};
inline SOCKET sock = INVALID_SOCKET;
inline SOCKET client_sock = INVALID_SOCKET;
inline KGB::div<SOCKET>all_clients_sockets;
inline KGB::div<user>user_list;
constexpr int frame_rate = 1000 / 480;
/*<----------------PROTOTYPES------------------->*/
std::optional<int>get_pos_in_str(std::string str, std::string sub_str);
std::optional<user> get_user_by_id(int id);
std::optional<user> get_user_by_socket(SOCKET id);
void send_message(user u_data, std::string msg, SOCKET s);
void send_message_all(user u_data, std::string msg);
void handler(const SOCKET s);
void new_user_handler();
void clear_old_socket_in_list(const SOCKET s);
bool create_socket( SOCKET* s, const std::string ip, const std::string port, bool TCP = true);
/*<----------------CODE------------------->*/
void handler(const SOCKET s) {
	 char recvbuf[DEFAULT_BUFLEN];
	 int recvbuflen = DEFAULT_BUFLEN;
	 int sz = 0;
	 do {		
		 send(s, "", 1, 0);
		 sz = recv(s, recvbuf, recvbuflen, 0);
		 if (sz > 0)
		 {
			 if (strstr(recvbuf, CREATE_USER))
			 {
				 std::optional<int>tmp = get_pos_in_str(recvbuf, CREATE_USER);			
				 if (tmp != std::nullopt)
				 {					
					 std::string name = "";
					 for (int i = 0; i < tmp.value(); i++)
					 {
						 name += recvbuf[i];
					 }					
					 user_list.emplace_back(user( name, s ));					
				 }
				 std::cout << "NEW USER HAS BEEN REGISTRATED:\n";
			 }
			 if (strstr(recvbuf, MESSAGE))
			 {
				 std::string message = "";
				 char* end_of_msg = strstr(recvbuf, MESSAGE);
				 for (const char& tmp : recvbuf)
				 {
					 if (&tmp == end_of_msg)break;
					 message += tmp;
				 }
				 if (message == "")continue;				 
				 std::optional<user> data = get_user_by_socket(s);
				 if (data != std::nullopt)
				 {
					 send_message_all(data.value(), message);
				 }
			 }
			 if (recvbuf[0] != '\0' && recvbuf[0] != 32)std::cout << recvbuf << std::endl;
			 if(sz > 4) ZeroMemory(&recvbuf, 512);
		 }
		 else {
			 std::cout << "CONNECTION WITH SOCKET ID: "<<s<<" LOST" << std::endl;
		 }
		 std::this_thread::sleep_for(std::chrono::milliseconds(frame_rate));
	 } while (sz > 0);
	 clear_old_socket_in_list(s);
	 for (int i = 0; i < user_list.length(); i++)
	 {
		 const user& u_tmp = user_list.at(i);
		 if (u_tmp.attached == s)user_list.erase(i);		
	 }
 }
int main(int argc, char** argv)
{
	user_list.reserve(100);
	if (!create_socket(&sock, SERVER_IP, SERVER_PORT))
	{
		std::cout << "\n << Error in creating socket. Server will shutdown << \n";
		return -1;
	}
	std::thread new_user_event_handler(new_user_handler);
	new_user_event_handler.join();
	int rez = shutdown(client_sock, SD_SEND);
	if (client_sock == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(sock);
		WSACleanup();
		return 1;
	}
	closesocket(client_sock);
	WSACleanup();
}
std::optional<user> get_user_by_id(const int id)
{
	/*for (const user& tmp : user_list)
	{
		if (tmp.unique_id == id)return tmp;
	}*/
	return std::nullopt;
}
std::optional<user> get_user_by_socket(SOCKET id)
{
	for (const user& tmp : user_list)
	{
		if (tmp.attached == id)return tmp;
	}
	return std::nullopt;
}
void send_message(const user u_data, const std::string msg, const SOCKET s)
{
	std::string tmp = "";
	tmp += u_data.name;
	tmp += " >> ";
	tmp += msg;
	tmp += SERVER_MSG;
	send(s, tmp.c_str(), tmp.length(), 0);
}
std::optional<int>get_pos_in_str(const std::string str, const std::string sub_str) {
	std::string buffer = "";
	int i = 0;
	for (i = 0; i < str.length() - sub_str.length(); i++)
	{
		for (char b : sub_str)
		{
			buffer += b;
		}
		int counter = 0;
		for (int j = 0; j < sub_str.length(); j++)
		{
			if (str[i + j] == buffer[j])
			{
				counter++;
			}
		}
		if (counter == sub_str.length())
		{
			return i;
			break;
		}
	}
	return std::nullopt;
}
void send_message_all(const user u_data, const std::string msg)
{
	std::string tmp = u_data.name +  ":: " + msg;
	tmp += SERVER_MSG;
	for (const SOCKET& a : all_clients_sockets)
		send(a, tmp.c_str(), tmp.length(), 0);
}
void new_user_handler() {
	while (true) {
		client_sock = accept(sock, NULL, NULL);
		if (client_sock != SOCKET_ERROR && client_sock != 0)
		{
			all_clients_sockets.emplace_back(client_sock);
			std::thread t(handler, client_sock);
			t.detach();
			std::cout << "NEW CONNECTION, SOCKET ID >>" << client_sock << std::endl;
		}
		else {
			std::cout << "ERROR IN ACCEPT CONNECTION >> " << WSAGetLastError() << std::endl;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(frame_rate));
	}
}
void clear_old_socket_in_list(const SOCKET s)
{
	for (int i = 0; i < all_clients_sockets.length(); i++) {
		const SOCKET& sock_link = all_clients_sockets.at(i);
		if (sock_link == s)all_clients_sockets.erase(i);
	}
}
bool create_socket(SOCKET* s, std::string ip, std::string port, bool TCP)
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
	hints.ai_socktype = SOCK_STREAM;//TCP
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	int rez = getaddrinfo(SERVER_IP, SERVER_PORT, &hints, &result);
	if (rez == EAI_AGAIN || rez == EAI_BADFLAGS || rez == EAI_FAIL)
	{
		printf("getaddrinfo failed  with error: %d\n", rez);
		return false;
	}
	*s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (sock == INVALID_SOCKET)
	{
		printf("SOCKET ARE NULL: ");
		WSACleanup();
		return false;
	}
	if (bind(*s, result->ai_addr, result->ai_addrlen) == SOCKET_ERROR)
	{
		std::cout << "Unable to bind the socket!\r\n";
		WSACleanup();
		system("PAUSE");
		return false;
	}
	freeaddrinfo(result);
	rez = listen(*s, SOMAXCONN);//начинаем прослушивать сокет
	if (rez == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(*s);
		WSACleanup();
		return false;
	}
	return true;
}
