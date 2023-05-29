#include "Client.h"

Client::Client(PCSTR address, PCSTR port)
{
    _address = address;
    _port = port;
}

void Client::Initialize()
{
    _networkManager.ResolveAddress(_address, _port);
    _networkManager.CreateParentSocket();
}

void Client::Connect()
{
    _networkManager.ConnectToServer();
}

void Client::Send(std::string message)
{
    _networkManager.Send(message);
}

std::string Client::Receive()
{
    std::string message(_networkManager.Receive());
    return message;
}

void Client::Close()
{
    _networkManager.CloseConnection();
}

void Client::PrintIPAddresses()
{
    _networkManager.PrintAddressInfo();
}
