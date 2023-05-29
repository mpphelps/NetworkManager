#pragma once
#include "NetworkManager.h"

class Server
{
private:
    NetworkManager _networkManager;
    PCSTR _port;
public:
    Server(PCSTR port);
    void Initialize();
    void WaitForConnection();
    void Send(std::string message, SOCKET socket);
    std::string Receive(SOCKET socket);
    void Close();
    std::vector<SOCKET> PollConnections();
};

