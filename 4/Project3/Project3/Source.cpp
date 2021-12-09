// для разрыва соединения клиент должен ввести строку "quit"
#include <iostream>
#include <winsock2.h>
#include <string>
#include <cstring>
using namespace std;
#pragma comment(lib,"Ws2_32.lib")
#pragma warning(disable:4996) //подавление предупреждения 4996
#define _WINSOCK_DEPRECATED_NO_WARNINGS
u_short MY_PORT = 666; // Порт, который слушает сервер
					   // макрос для печати количества активных пользователей
#define PRINTNUSERS if(nclients)\
cout << "Online users: " << nclients << endl;\
else cout << "No user online\n";
					   // прототип функции, обслуживающий  подключившихся клиентов
DWORD WINAPI ConToClient(LPVOID client_socket);
// глобальная переменная – количество  активных пользователей
int nclients = 0;
const char ANSWER1[] = "you are in arrears";
const char ANSWER2[] = "you don't have any debts but you can't get a scholarship";
const char ANSWER3[] = "you can receive a scholarship of 2200 rubles";
const char ANSWER4[] = "you can receive a scholarship of 1700 rubles";

int order = 1;
string ANSWER = "Prise: ";

struct get_price {
	char name[30];
	string skin;
	string make_up;
};

int complexity(string skin) {
	if (skin[0] == 'A') return 2;
	if (skin[0] == 'C') return 1;
	if (skin[0] == 'D') return 1;
	if (skin[0] == 'B') return 2;
	return 0;
}

int choice_mu(string make_up) {
	if (make_up[0] == 'C') return 4000;
	if (make_up[0] == 'A') return 1500;
	if (make_up[0] == 'B') return 2500;
	return 0;
}

int cost(get_price  client) {
	return  complexity(client.skin) * choice_mu(client.make_up);
}

int main() {
	char buff[1024];// Буфер для различных нужд
	cout << "TCP SERVER DEMO\n";

	// Шаг 1 - Инициализация Библиотеки Сокетов
	if (WSAStartup(0x0202, (WSADATA*)&buff[0])) {         // Ошибка!
		cout << "Error WSAStartup\n" << WSAGetLastError();
		return -1;
	}

	// Шаг 2 - создание сокета
	SOCKET mysocket;
	if ((mysocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {    // Ошибка!
		cout << "Error socket\n" << WSAGetLastError();
		WSACleanup();// Деиницилизация библиотеки Winsock
		return -1;
	}

	// Шаг 3 связывание сокета с локальным адресом
	sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(MY_PORT);
	local_addr.sin_addr.s_addr = 0;

	// вызываем bind для связывания
	if (bind(mysocket, (sockaddr*)&local_addr, sizeof(local_addr))) {
		// Ошибка
		cout << "Error bind\n" << WSAGetLastError();
		closesocket(mysocket);  // закрываем сокет!
		WSACleanup();
		return -1;
	}

	// Шаг 4 ожидание подключений
	// размер очереди – 0x100
	if (listen(mysocket, 0x100)) {
		// Ошибка
		cout << "Error listen: " << WSAGetLastError();
		closesocket(mysocket);
		WSACleanup();
		return -1;
	}

	cout << "Waiting connections\n";

	// Шаг 5 извлекаем сообщение из очереди
	SOCKET client_socket;// сокет для клиента
	sockaddr_in client_addr;// адрес клиента

							// функции accept необходимо передать размер структуры
	int client_addr_size = sizeof(client_addr);

	// цикл извлечения запросов на подключение из очереди
	while ((client_socket = accept(mysocket, (sockaddr*)
		&client_addr, &client_addr_size))) {
		nclients++;   // увеличиваем счетчик  клиентов
		HOSTENT* hst;  // пытаемся получить имя хоста
		hst = gethostbyaddr((char*)&client_addr.sin_addr.s_addr, 4, AF_INET);
		cout << "+new connect!\n";    // вывод сведений о клиенте
		if (hst)
			cout << hst->h_name << ' ';
		else cout << "";
		cout << inet_ntoa(client_addr.sin_addr) << endl;
		PRINTNUSERS cout << endl;

		// Вызов нового потока для обслуживания клиента
		DWORD thID;
		CreateThread(NULL, NULL, ConToClient, &client_socket, NULL, &thID);
	}

	return 0;
}
/* Эта функция создается в отдельном потоке и обслуживает очередного подключившегося клиента независимо от остальных */
DWORD WINAPI ConToClient(LPVOID client_socket) {
	SOCKET my_sock;
	get_price  customer;
	int len;
	my_sock = ((SOCKET*)client_socket)[0];
	char buff[1024];
	char sHELLO[] = "Welcome to KUBSU\r\n";

	send(my_sock, sHELLO, sizeof(sHELLO), 0); // отправляем клиенту приветствие

											  // цикл эхо-сервера: прием строки от клиента и  возвращение ее клиенту
	while (SOCKET_ERROR != (len = recv(my_sock, (char*)&customer, sizeof(get_price), 0))) {
		if (strcmp(customer.name, "quit") == 0)
			break;
		else {
			cout << endl << "Order #" << order << endl;
			order++;
			cout << "Name: " << customer.name << endl;
			cout << "Subj: " <<  customer.skin << endl;
			cout << "Mark: " << customer.make_up << endl;
			int l = 0;
			for (int i = 0; customer.name[i] != '\0'; i++) {
				buff[i] = customer.name[i];
				l++;
			}
			buff[l] = ',';
			buff[l + 1] = ' ';
			string msg = ANSWER;
			int a = cost(customer);
			cout << "Price: " << a << " rub" << endl;
			msg += to_string(a);
			msg += " rub.";
			for (int i = l + 2, j = 0; msg[j] != '\0'; i++, j++) {
				buff[i] = msg[j];
				l = i;
			}
			buff[l + 1] = '\0';

			send(my_sock, (char*)&buff, l + 1, 0);
		}
	}

	// произошел выход из цикла, соединение c клиентом разорвано
	nclients--;// уменьшаем счетчик активных клиентов
	cout << "Client disconnect\n";
	PRINTNUSERS
		closesocket(my_sock);
	return 0;
}