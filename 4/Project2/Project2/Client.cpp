#include <winsock2.h>
#include <iostream>
#include <string>
#define PORT 666
#define SERVERADDR "127.0.0.1"
#pragma comment(lib,"Ws2_32.lib")
#pragma warning(disable:4996) //���������� �������������� 4996
#define _WINSOCK_DEPRECATED_NO_WARNINGS
using namespace std;

const int N_QUEST = 3;
const string QUEST[N_QUEST] = { "Enter your name ", "Enter your subject (A - seti, B - web, C - dif, D - oop): ", "What mark do you want (A - 5, B - 6, C - 7): " };

struct get_price {
	char name[30];
	string skin;
	string make_up;
};

int main() {
	setlocale(0, "");
	char buff[1024];
	cout << "TCP DEMO CLIENT\n";

	// ��� 1 - ������������� ���������� Winsock
	if (WSAStartup(0x202, (WSADATA*)&buff[0])) {
		cout << "WSAStart_error:\n" << WSAGetLastError();
		cin.get();
		return -1;
	}

	// ��� 2 - �������� ������
	SOCKET my_sock; my_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (my_sock < 0) {
		cout << "Socket()_error:\n" << WSAGetLastError();
		cin.get();
		return -1;
	}

	// ��� 3 - ��������� ����������
	// ���������� ��������� sockaddr_in, �������� ������ � ����� �������
	sockaddr_in dest_addr;
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(PORT);
	HOSTENT* hst;
	// �������������� IP ������ �� ����������� � ������� ������
	if (inet_addr(SERVERADDR) != INADDR_NONE)
		dest_addr.sin_addr.s_addr = inet_addr(SERVERADDR);
	else
		// ������� �������� IP ����� �� ��������� ����� �������
		if (hst = gethostbyname(SERVERADDR))
			((unsigned long*)&dest_addr.sin_addr)[0] =
			((unsigned long**)hst->h_addr_list)[0][0];
		else {
			cout << "Invalid address: " << SERVERADDR;
			closesocket(my_sock);
			WSACleanup();
			cin.get();
			return -1;
		}

	// ����� ������� ������� � �������� ���������� ����������
	if (connect(my_sock, (sockaddr*)&dest_addr, sizeof(dest_addr))) {
		cout << "Connect error\n" << WSAGetLastError();
		cin.get();
		return -1;
	}

	cout << "Connection to " << SERVERADDR << " successfully established";
	cout << "\nType quit for exit\n\n";

	// ��� 4 - ������ � �������� ���������
	int len;
	while ((len = recv(my_sock, &buff[0], 90, 0)) != SOCKET_ERROR) {
		buff[len] = '\0';
		cout << "S=>C: ";
		cout << buff << "\n\n";

		// ������ ���������������� ���� � ����������
		get_price  customer;
		cout << "S<=C:\n";
		cout << QUEST[0];
		cin >> customer.name;
		if (strcmp(customer.name, "quit") == 0) { // �������� �� "quit"
			customer.name == "quit";
			send(my_sock, (char*)&customer, sizeof(get_price), 0);
			cout << "Exit..."; // ���������� �����
			closesocket(my_sock);
			WSACleanup();
			return 0;
		}
		cout << QUEST[1];
		cin >> customer.skin;
		cout << QUEST[2];
		cin >> customer.make_up;
		// �������� ������ ������� �������
		send(my_sock, (char*)&customer, sizeof(get_price), 0);
	}

	cout << "Recv() error\n" << WSAGetLastError();
	closesocket(my_sock);
	WSACleanup();
	cin.get();
	return -1;
}