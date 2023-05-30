#include "Server.h"

Server::Server(PCSTR port)
{
    _port = port;
    IsConnected = false;
    InSession = true;
}

void Server::Initialize()
{
    _networkManager.ResolveAddress(NULL, _port);
    _networkManager.CreateParentSocket();
    _networkManager.BindSocket();
    _networkManager.StartListening();
}

void Server::WaitForConnection()
{
    while (true) {
        if (HasIncomingConnection())
        {
            AcceptConnection();
            break;
        }
        else
        {
            Sleep(1000);
        }
    }
}

void Server::AcceptConnection()
{
    _networkManager.AcceptConnection();
    IsConnected = true;
}

void Server::Send(std::string message, SOCKET socket)
{
    _networkManager.Send(message, socket);
}

std::string Server::Receive(SOCKET socket)
{
    std::string message(_networkManager.Receive(socket));
    return message;
}

void Server::Close()
{
    _networkManager.CloseAllConnections();
    IsConnected = false;
    InSession = false;
}

bool Server::HasIncomingConnection()
{
    return _networkManager.PollIsParentReady(0);
}

std::vector<SOCKET> Server::PollConnections()
{
    return _networkManager.PollChildren(10000);
}


