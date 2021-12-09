#include <iostream>
#include <winsock2.h>
#include <Windows.h>
#include <string>
#include <ctime>
#include <conio.h>

#pragma warning(disable: 4996) 
#pragma comment (lib, "Ws2_32.lib")

using namespace std;

string message;
string nickName;

const short serverPort = 1234; // Порт сервера
const short clientPort = 1235; // Порт клиента

enum type_of_message {
    CONNECT,
    DISCONNECT,
    PRIVATE,
    ALL
};

struct Client {
    char nick[1024];
    type_of_message current;
    char private_nick[1024];
    char message[1024];
};

void initializeLibrary() {
    WSADATA wsaData;//содержит информацию о реализации сокетов Windows
    int errorCode = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (errorCode != 0) {
        cout << "Error initialization" << endl;
        exit(1);
    }
}

SOCKET createSocket() {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0); // Создает TCP сокет
    if (sock == INVALID_SOCKET) {
        cout << "Error socket creation" << endl;
        exit(1);
    }
    return sock;
}

void bindSocket(SOCKET sock) {
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = clientPort + rand() % 100;
    address.sin_addr.s_addr = 0;
    int errorCode = bind(sock, (sockaddr*)&address, sizeof(address));
    if (errorCode != 0) {
        cout << "Error bind" << endl;
        exit(1);
    }
}

void connectSocket(SOCKET sock) {
    hostent* hostentStruct;
    hostentStruct = gethostbyname("localhost");

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = serverPort;

    memcpy(&address.sin_addr.s_addr,
        hostentStruct->h_addr_list[0],
        sizeof(address.sin_addr.s_addr)); // Заполняет address.sin_addr.s_addr из hostentStruct
    int errorCode = connect(sock, (sockaddr*)&address, sizeof(address));
    if (errorCode != 0) {
        cout << "Error connection" << endl;
        exit(1);
    }
}

string readResponse(SOCKET sock) {
    char buffer[1024];
    int bytes = recv(sock, buffer, sizeof(buffer), 0);
    if (bytes == 0) { // Если клиент отсоединился
        cout << "Welcome to the chat. Commands: '-leave'(to leave) '-private'(private massege).\n" << endl;
        closesocket(sock);
        exit(0);
    }
    else if (bytes < 0) {
        cout << "Welcome to the chat. Commands: '-leave'(to leave) '-private'(private massege).\n" << endl;
        closesocket(sock);
        exit(0);
    }
    else return string(buffer);
}

// Вернет true, если клиент отсоединился
bool writeMessage(SOCKET sock, Client forMessage) {
    Client example = forMessage;
    int bytes = send(sock, (char*)&example, sizeof(example), 0);
    if (bytes < 0) {
        cout << "ERROR" << endl;
        return true;
    }
    else return bytes == 0;
}

Client createMessageStruct(string nick, type_of_message current, string private_nick, string message) {
    Client result;
    strcpy(result.nick, nick.c_str());
    result.current = current;
    strcpy(result.private_nick, private_nick.c_str());
    strcpy(result.message, message.c_str());
    return result;
}

bool accessByNick(SOCKET sock) {
    Client forConnect = createMessageStruct(nickName, CONNECT, "", "");
    writeMessage(sock, forConnect);

    string accessResponse = readResponse(sock);

    cout << accessResponse << endl;
    if (accessResponse == "Nick has already been taken") {
        closesocket(sock);
        return false;
    }
    else return true;
}

CRITICAL_SECTION cs;

DWORD WINAPI ThreadWork(LPVOID lpParameter) {//DWORD - 32битовое беззнаковое целое, LPVOID lpParametr- данные потока
    SOCKET sock = *(SOCKET*)lpParameter;
    while (true) {
        string response = readResponse(sock);
        cout << '\r' << response;
        if (response.length() < message.length())
            for (int i = 0; i < message.length() - response.size(); i++)
                cout << ' ';
        cout << endl << message;
    }
    return 0;
}

void Chatting(SOCKET sock) {
    string nickFormat = nickName + ": ";
    Client forAccess = createMessageStruct("", ALL, "", nickName + " join the chat.\n");
    writeMessage(sock, forAccess);
    while (true) {
        Client forMessage;
        cout << nickFormat;
        message += nickFormat;
        while (true) {
            char symbol_example = getch(); //getch() возвращает очередной символ, считанный с консоли, но не выводит этот сим­вол на экран
            if (symbol_example == '\r') {
                cout << endl;
                break;
            }
            if (symbol_example == '\b') {
                cout << '\r';
                for (int i = 0; i < message.length(); i++)
                    cout << ' ';
                if (message.size() > nickFormat.size())
                    message.erase(message.length() - 1);
                cout << '\r' << message;
            }
            else {
                cout << symbol_example;
                message += symbol_example;
            }
        }
        if (message.find("-leave") != string::npos) {
            forMessage = createMessageStruct(nickName, DISCONNECT, "", "");
            writeMessage(sock, forMessage);
        }
        else if (message.find("-private") != string::npos) {
            string private_nick = "";
            int from = message.find("-private") + 20;
            int temp = from - 20;
            while (message[from] != ' ') {
                private_nick += message[from];
                from++;
            }
            while (message[temp] != ' ') {
                message.erase(temp, 1);
            }
            forMessage = createMessageStruct(nickName, PRIVATE, private_nick, message);
            writeMessage(sock, forMessage);
            message.clear();
        }
        else {
            forMessage = createMessageStruct(nickName, ALL, "", message);
            writeMessage(sock, forMessage);
            message.clear();
        }
    }
}

int main() {
    setlocale(LC_ALL, 0);
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    srand(time(NULL));

    SOCKET sock;

    initializeLibrary();
    InitializeCriticalSection(&cs);

    while (true) {
        cout << "Enter your name: ";
        getline(cin, nickName);


        sock = createSocket();
        bindSocket(sock);
        cout << "Waiting for connection..." << endl;
        connectSocket(sock);

        if (accessByNick(sock))
            break;
    }

    CreateThread(NULL, 0, &ThreadWork, &sock, 0, NULL);
    Chatting(sock);
}