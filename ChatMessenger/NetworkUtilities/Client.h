#pragma once
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include "../../ChatServer/ChatServer/NetworkManager.h"

class Client
{
private:
    NetworkManager _networkManager;
    PCSTR _address;
    PCSTR _port;
public:
    bool IsConnected;
public:
    Client(PCSTR address, PCSTR port);
    void Initialize();
    void Connect();
    void Send(std::string message);
    std::string Receive();
    void Close();
    void PrintIPAddresses();
    std::string GetUserMessage();
};

