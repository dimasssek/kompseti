//www-client.cpp
#include <math.h>
#include <conio.h>
#include <stdio.h>
#include <locale.h>
#include <string>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>
#pragma comment(lib,"Ws2_32.lib")
#pragma warning(disable:4996) //подавление предупреждения 4996
using namespace std;
#define request "GET / HTTP/1.1\r\nHOST: about.gitlab.io\r\n\r\n" //html запрос
#define max_packet_size 65535
int main() {
	WSADATA ws;
	SOCKET s;
	sockaddr_in adr;
	HOSTENT* hn;
	char buff[max_packet_size];
	//инициализация
	if (WSAStartup(0x0202, &ws) != 0) return -1; //error
	//создание сокета
	if (INVALID_SOCKET == (s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))) return -1; //если ошибка
	//получение адреса
	if (NULL == (hn = gethostbyname("www.m.kuzmenko.gitlab.io"))) return -1; //если ошибка
	//заполнение структуры
	adr.sin_family = AF_INET;
	adr.sin_addr.s_addr = *((unsigned long*)hn->h_addr);
	adr.sin_port = htons(80);
	//уставновление соединения
	if (SOCKET_ERROR == connect(s, (SOCKADDR*)&adr, sizeof(adr))) {
		cout << "here1" << endl;
		int res = WSAGetLastError();
		return -1;
	}//обработка ошибки соединения
	// посылка запроса серверу
	if (SOCKET_ERROR == send(s, (char*)&request, sizeof(request), 0)) {
		cout << "here2" << endl;
		int res = WSAGetLastError();
		return -1;
	}
	int len = 0;
	//ожидание ответа в цикле
	do {
		if (SOCKET_ERROR == (len = recv(s, (char*)&buff, max_packet_size, 0))) {
			cout << "here3" << endl;
			int res = WSAGetLastError();
			return -1;
		}
		//вывод ответа
		for (int i = 0; i < len; i++) cout << buff[i];
	} while (len != 0);
	//закрытие соединения
	if (SOCKET_ERROR == closesocket(s)) return -1;
	cin.get();
	return 1;
}