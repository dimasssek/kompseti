//www-client.cpp

#include <math.h>
#include <conio.h>
#include <stdio.h>
#include <locale.h>
#include <string>
#include <winsock2.h>
#include <iostream>
#pragma comment(lib,"Ws2_32.lib")
#pragma warning(disable:4996) //подавление предупреждения 4996
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define request "get /?name=request1&type=3&amount=1 http/1.1\r\n host: localhost \r\n\r\n" //html запрос
#define max_packet_size 65535
using namespace std;

int main() {
	WSADATA ws;
	SOCKET s;
	sockaddr_in adr;
	HOSTENT* hn;
	char buff[max_packet_size];
	//инициализация
	if (WSAStartup(0x0202, &ws) != 0) {
		return -1; //error
	}

	//создание сокета
	if (INVALID_SOCKET == (s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))) {
		return -1; //если ошибка
	}

	//получение адреса
	if (NULL == (hn = gethostbyname("localhost"))) {
		return -1; //если ошибка
	}

	//заполнение структуры
	adr.sin_family = AF_INET;
	((unsigned long*)&adr.sin_addr)[0] =
		((unsigned long**)hn->h_addr_list)[0][0];
	adr.sin_port = htons(8000);

	//уставновление соединения
	if (SOCKET_ERROR == connect(s, (SOCKADDR*)&adr, sizeof(adr))) {
		int res = WSAGetLastError();
		return -1;
	}//обработка ошибки соединения

	// посылка запроса серверу
	if (SOCKET_ERROR == send(s, (char*)&request, sizeof(request), 0)) {
		int res = WSAGetLastError();
		return -1;
	}

	//Ждем ответа
	int len = 0;
	do {
		if (SOCKET_ERROR == (len = recv(s, (char*)&buff, max_packet_size, 0))) {
			int res = WSAGetLastError();
			return -1;
		}

		for (int i = 0; i < len; i++) cout << buff[i];
	} while (len != 0);

	//закрытие соединения
	if (SOCKET_ERROR == closesocket(s)) {
		return -1;
	}

	cin.get(); cin.get();
	return 1;
}