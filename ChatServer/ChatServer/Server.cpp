#include "Server.h"

Server::Server(PCSTR port)
{
    _port = port;
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
    _networkManager.AcceptConnection();
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
}

std::vector<SOCKET> Server::PollConnections()
{
    return _networkManager.PollSockets();
}


