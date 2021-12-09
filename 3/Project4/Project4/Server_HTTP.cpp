//www-server.cpp
#include <math.h>
#include <conio.h>
#include <stdio.h>
#include <locale.h>
#include <iostream>
#include <sstream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment (lib, "WS2_32.lib")
using namespace std;
using std::cerr;
int main() {
	WSADATA wsaData; // служебна€ структура дл€ хранени€ информации
	// о реализации Windows Sockets
	// старт использовани€ библиотеки сокетов процессом
	// (подгружаетс€ Ws2_32.dll)
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		// ≈сли произошла ошибка загрузки библиотеки
		cerr << "WSAStartup failed: " << result << endl;
		return result;
	}

	struct addrinfo* addr = NULL;
	// структура дл€ IP адреса сервера (слушающего сокета)
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE; // бинт на адрес дл€ прин€ти€ вход€щих соединений
	//инициализаци€ addr
	//создаем сервер на 8000 порту
	result = getaddrinfo("127.0.0.1", "8000", &hints, &addr);
	// ≈сли инициализаци€ структуры адреса завершилась с ошибкой,
	if (result != 0) {
		cerr << "getaddrinfo failed: " << result << endl;
		WSACleanup();
		return 1;
	}

	//создание сокета
	int listenSocket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if (listenSocket == INVALID_SOCKET) {
		// ≈сли создание сокета завершилось с ошибкой
		cerr << "Error at socket: " << WSAGetLastError() << endl;
		freeaddrinfo(addr);
		WSACleanup();
		return 1;
	}

	//прив€зываем сокет к IP адресу
	result = bind(listenSocket, addr->ai_addr, (int)addr->ai_addrlen);
	if (result == SOCKET_ERROR) {
		// ≈сли прив€зать адрес к сокету не удалось
		cerr << "bind failed with error: " << WSAGetLastError() << endl;
		freeaddrinfo(addr);
		WSACleanup();
		return 1;
	}

	//инициализаци€ слушающего сокета
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		cerr << "listen failed with error: " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	const int maxClientBufferSize = 1024;
	char buf[maxClientBufferSize];
	int clientSocket = INVALID_SOCKET;
	//принимаем вход€щие соединени€
	for (;;) {
		clientSocket = accept(listenSocket, NULL, NULL);
		if (clientSocket == INVALID_SOCKET) {
			cerr << "accept failed: " << WSAGetLastError() << endl;
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}
		result = recv(clientSocket, buf, maxClientBufferSize, 0);
		stringstream response;
		//ответ клиенту
		stringstream response_body;
		//тело ответа

		if (result == SOCKET_ERROR) {
			//ошибка получени€ данных
			cerr << "recv failed: " << result << endl;
			closesocket(clientSocket);
		}
		else if (result == 0) {
			//соединение закрыто клиентом
			cerr << "connection closed" << endl;
		}
		else if (result > 0) {
			// ћы знаем фактический размер полученных данных, поэтому ставим метку конца строки
			buf[result] = '\0'; //данные успешно получены
			//формируем тело ответа
			response_body << "<title> C++ Test HTTP Server </title> \n"
				<< "<h1> Khlybov is testing  </h1> \n"
				<< "<p> Lorem ipsum dolor sit amet...  </p>\n"
				<< "<h2> Request headers </h2>\n"
				<< "<pre>" << buf << "</pre>\n"
				<< "<em><small> Test is successful! </small></em>\n";
			//формируем ответ
			response << "HTTP/1.1 200 OK\r\n"
				<< "Version: HTTP/1.1\r\n"
				<< "Content-type: text/html; charset = utf-8\r\n"
				<< "Content-Length: " << response_body.str().length()
				<< "\r\n\r\n" << response_body.str();

			cout << buf << endl;
			//отправл€ем ответ клиенту
			result = send(clientSocket, response.str().c_str(), response.str().length(), 0);
			if (result == SOCKET_ERROR) {
				//ошибка при отправке
				cerr << "send failed: " << WSAGetLastError() << endl;
			}

			closesocket(clientSocket);
		}
	}
	//полное закрытие
	closesocket(listenSocket);
	freeaddrinfo(addr);
	WSACleanup();
	return 0;
}