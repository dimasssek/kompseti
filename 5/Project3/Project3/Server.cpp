#include <iostream>
#include <winsock2.h>
#include <Windows.h>
#include <string>
#include <vector>

#pragma warning(disable: 4996) 
#pragma comment (lib, "Ws2_32.lib")

using namespace std;

CRITICAL_SECTION cs;
vector <pair<string, SOCKET>> sockStore(0);

const short serverPort = 1234; // ѕорт сервера

enum typeOfMessage {
    CONNECT,
    DISCONNECT,
    PRIVATE,
    ALL
};

struct Client {
    char nick[1024];
    typeOfMessage current;
    char privateNick[1024];
    char message[1024];
};

void initializeLibrary() {
    WSADATA wsaData;//содержит информацию о реализации сокетов Windows
    int errorCode = WSAStartup(MAKEWORD(2, 2), &wsaData); // MAKEWORD(2, 2) == 0x0202 - верси€ 2.2
    if (errorCode != 0) {
        cout << "Error initialization" << endl;
        exit(1);
    }
}

SOCKET createSocket() {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0); // —оздает TCP сокет
    if (sock == INVALID_SOCKET) {
        cout << "Error socket creation" << endl;
        exit(1);
    }
    return sock;
}

void bindSocket(SOCKET sock) {
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = serverPort;
    address.sin_addr.s_addr = 0;
    int errorCode = bind(sock, (sockaddr*)&address, sizeof(address));
    if (errorCode != 0) {
        cout << "Error bind" << endl;
        exit(1);
    }
}

SOCKET acceptConnection(SOCKET listeningSock) {
    sockaddr_in clientAddress;
    int clientAddressSize = sizeof(clientAddress);
    SOCKET result = accept(listeningSock, (sockaddr*)&clientAddress, &clientAddressSize);
    // ƒелаем что то с адресом клиента clientAddress
    // ≈сли не нужно, можно просто сделать accept(listeningSock, NULL, NULL)
    HOSTENT* hst;
    hst = gethostbyaddr((char*)&clientAddress.sin_addr, 4, AF_INET);
    cout << "New connection: ";
    cout << ((hst) ? hst->h_name : "Unknown host") << "/" << inet_ntoa(clientAddress.sin_addr) << "/" << ntohs(clientAddress.sin_port) << '\n';
    //inet_ntoa преобразует сетевой адрес »нтернета (Iv4) в строку ASCII
    //ntohs преобразует u_short из сетевого пор€дка байтов TCP / IP в пор€док байтов хоста
    return result;
}

Client clearClient(Client example) {
    example.nick[0] = '\n';
    example.current = DISCONNECT;
    example.privateNick[0] = '\n';
    example.message[0] = '\n';
    return example;
}

Client readRequest(SOCKET sock) {
    Client buffer;
    int bytes = recv(sock, (char*)&buffer, sizeof(buffer), 0);
    if (bytes == 0) { // ≈сли клиент отсоединилс€
        buffer = clearClient(buffer);
        return buffer;
    }
    else if (bytes < 0) {
        cout << "ERROR" << endl;
        buffer = clearClient(buffer);
        return buffer;
    }
    else return buffer;
}

bool alreadyExist(string nick) {
    for (int i = 0; i < sockStore.size(); i++) {
        if (sockStore[i].first == nick)
            return true;
    }
    return false;
}

// ¬ернет true, если клиент отсоединилс€
bool writeResponse(SOCKET sock, string str) {
    int bytes = send(sock, str.c_str(), str.length() + 1, 0);//c_str - возвращает символ const char* , указывающий на строку с завершением null
    if (bytes < 0) {
        cout << "ERROR" << endl;
        return true;
    }
    else return bytes == 0;
}

DWORD WINAPI ThreadWork(LPVOID lpParameter) {
    SOCKET clientSock = *(SOCKET*)lpParameter;
    while (true) {
        Client request = readRequest(clientSock);
        EnterCriticalSection(&cs);
        if (request.current == CONNECT) {
            if (alreadyExist(string(request.nick))) {
                writeResponse(clientSock, "Nick has already been taken");
                closesocket(clientSock);
                LeaveCriticalSection(&cs);
                break;
            }
            else {
                string list_of_members = "";
                if (!sockStore.empty()) {
                    list_of_members += "\nOnline:\n";
                    for (pair<string, SOCKET> example : sockStore) {
                        list_of_members += example.first;
                        list_of_members += "\n";
                    }
                }
                writeResponse(clientSock, "Welcome to the chat. Commands: '-leave'(to leave) '-private'(private massege).\n" + list_of_members);
                sockStore.push_back(make_pair(string(request.nick), clientSock));
            }
        }
        else if (request.current == DISCONNECT) {
            closesocket(clientSock);
            sockStore.erase(std::find(sockStore.begin(), sockStore.end(), make_pair(string(request.nick), clientSock)));
            for (int i = 0; i < sockStore.size(); i++) {
                writeResponse(sockStore[i].second, string(request.nick) + " leave the chat.\n");
            }
            LeaveCriticalSection(&cs);
            break;
        }
        else if (request.current == ALL) {
            for (int i = 0; i < sockStore.size(); i++) {
                if (clientSock != sockStore[i].second)
                    writeResponse(sockStore[i].second, string(request.message));
            }
        }
        else if (request.current == PRIVATE) {
            for (int i = 0; i < sockStore.size(); i++) {
                if (sockStore[i].first == string(request.privateNick))
                    writeResponse(sockStore[i].second, "PRIVATE_MESSAGE_FROM " + string(request.message));
            }
        }
        LeaveCriticalSection(&cs);
    }
    return 0;
}

int main() {

    setlocale(LC_ALL, 0);
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    initializeLibrary();
    InitializeCriticalSection(&cs);

    SOCKET listeningSock = createSocket();
    bindSocket(listeningSock);

    listen(listeningSock, SOMAXCONN);
    cout << "Listening..." << endl;
    while (true) {
        SOCKET clientSock = acceptConnection(listeningSock);
        CreateThread(NULL, 0, &ThreadWork, &clientSock, 0, NULL);
    }
}