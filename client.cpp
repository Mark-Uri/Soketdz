#include <iostream>
#include <winsock2.h>
#include <thread>
#include <string>

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)

using namespace std;

#define SERVER "127.0.0.1"
#define PORT 8888
#define BUFLEN 512

SOCKET client_socket;




void receive()
{
    char buffer[BUFLEN];
    while (true)
    {
        int len = recv(client_socket, buffer, BUFLEN - 1, 0);


        if (len > 0) 
        {
            buffer[len] = '\0';
            cout << buffer << endl;
        }
    }
}



void showMenu(bool isAdmin)
{
    cout << "\n===== МЕНЮ =====\n";
    cout << "1. Присоединиться к общей комнате\n";
    cout << "2. Войти в приватную комнату\n";
    cout << "3. Написать личное сообщение\n";
    cout << "4. Посмотреть историю\n";
    cout << "5. Выйти\n";
    if (isAdmin) 
    {
        cout << "6. Удалить пользователя\n";
        cout << "7. Забанить пользователя\n";
        cout << "8. Просмотр логов с цензурой\n";
    }
    cout << "Выберите действие: ";
}



int main() 
{


    setlocale(LC_ALL, "");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);




    WSAData wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);




    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(SERVER);



    connect(client_socket, (sockaddr*)&server, sizeof(server));


    cout << "Введите ваш ник: ";
    string nickname;
    getline(cin, nickname);


    send(client_socket, nickname.c_str(), nickname.length(), 0);

    bool isAdmin = (nickname == "admin");

    thread listener(receive);




    while (true) 
    {
        showMenu(isAdmin);
        int choice;
        cin >> choice;
        cin.ignore();

        string msg;



        switch (choice)
        {
        case 1:
            send(client_socket, "JOIN::main", 10, 0);
            break;
        case 2:

            cout << "Введите название комнаты: ";
            getline(cin, msg);
            msg = "JOIN::" + msg;


            send(client_socket, msg.c_str(), msg.length(), 0);


            break;
        case 3: 
        {
            cout << "Введите получателя: ";
            string to;
            getline(cin, to);
            cout << "Введите сообщение: ";
            getline(cin, msg);


            msg = "PRIVATE::" + to + "::" + msg;
            send(client_socket, msg.c_str(), msg.length(), 0);
            break;
        }
        case 4:
            send(client_socket, "HISTORY", 7, 0);
            break;
        case 5:
            send(client_socket, "EXIT", 4, 0);
            exit(0);

            break;
        case 6:
            if (isAdmin) 
            {
                cout << "Введите ник для удаления: ";
                getline(cin, msg);
                msg = "ADMIN::DELETE::" + msg;

                send(client_socket, msg.c_str(), msg.length(), 0);
            }
            break;
        case 7:
            if (isAdmin)
            {
                cout << "Введите ник для бана: ";
                getline(cin, msg);
                msg = "ADMIN::BAN::" + msg;
                send(client_socket, msg.c_str(), msg.length(), 0);
            }
            break;
        case 8:
            if (isAdmin)
            {
                send(client_socket, "ADMIN::CENSOR_LOG", 17, 0);
            }
            break;
        default:
            cout << "Неверный выбор\n";
        }
    }

    listener.join();
    closesocket(client_socket);
    WSACleanup();
    
}
