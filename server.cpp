#include <iostream>
#include <winsock2.h>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <ctime>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)


using namespace std;

#define PORT 8888
#define BUFLEN 512
#define MAX_CLIENTS 10


SOCKET server_socket;
SOCKET client_sockets[MAX_CLIENTS] = {};
map<SOCKET, string> nicknames;
vector<string> censored_words = { "bad", "evil", "ugly" };



string timestamp() 
{
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char buf[64];


    sprintf(buf, "%02d:%02d:%02d", ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    return string(buf);
}

string censor(string msg)
{
    for (auto& word : censored_words) 
    {

        size_t pos;
        while ((pos = msg.find(word)) != string::npos)
        {

            msg.replace(pos, word.length(), "###");
        }
    }
    return msg;
}






void broadcast(string msg) 
{
    ofstream out("chat_history.txt", ios::app);
    out << msg << "\n";
    out.close();

    for (int i = 0; i < MAX_CLIENTS; i++) 
    {
        if (client_sockets[i] != 0) 
        {
            send(client_sockets[i], msg.c_str(), msg.length(), 0);
        }
    }
}

int main() 
{

    setlocale(LC_ALL, "");
    WSAData wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);



    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server;

    server.sin_family = AF_INET;

    server.sin_addr.s_addr = INADDR_ANY;


    server.sin_port = htons(PORT);




    bind(server_socket, (sockaddr*)&server, sizeof(server));
    listen(server_socket, MAX_CLIENTS);

    cout << "Сервер запущен Ожидание подключений...\n";

    fd_set readfds;

    while (true)
    {
        FD_ZERO(&readfds);

        FD_SET(server_socket, &readfds);
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (client_sockets[i] > 0) FD_SET(client_sockets[i], &readfds);
        }


        select(0, &readfds, NULL, NULL, NULL);



        if (FD_ISSET(server_socket, &readfds)) 
        {
            SOCKET new_socket = accept(server_socket, NULL, NULL);


            char nickname[BUFLEN] = {};
            recv(new_socket, nickname, BUFLEN, 0);


            nicknames[new_socket] = nickname;

            for (int i = 0; i < MAX_CLIENTS; i++) 
            {
                if (client_sockets[i] == 0)
                {

                    client_sockets[i] = new_socket;
                    break;
                }
            }

            
            ifstream in("chat_history.txt");
            string line;

            while (getline(in, line))
            {
                send(new_socket, line.c_str(), line.size(), 0);
                send(new_socket, "\n", 1, 0);
            }
            in.close();

            string welcome = string(nickname) + " присоединился к чату \n";

            broadcast(welcome);
        }

        for (int i = 0; i < MAX_CLIENTS; i++) 
        {
            SOCKET s = client_sockets[i];



            if (FD_ISSET(s, &readfds))
            {
                char buf[BUFLEN] = {};
                int valread = recv(s, buf, BUFLEN, 0);

                if (valread == SOCKET_ERROR || valread == 0) 
                {
                    closesocket(s);

                    client_sockets[i] = 0;
                    string quit = nicknames[s] + " вышел из чата";
                    broadcast(quit);
                    continue;
                }

                string msg = censor(buf);
                string full = nicknames[s] + " [" + timestamp() + "]: " + msg;

                broadcast(full);
            }
        }


    }

    WSACleanup();
   
}
