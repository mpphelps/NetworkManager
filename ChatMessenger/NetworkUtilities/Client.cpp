#include "Client.h"

Client::Client(PCSTR address, PCSTR port)
{
    _address = address;
    _port = port;
    IsConnected = false;
}

void Client::Initialize()
{
    _networkManager.ResolveAddress(_address, _port);
    _networkManager.CreateParentSocket(); 
}

void Client::Connect()
{
    _networkManager.ConnectToServer();
    IsConnected = true;
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
    IsConnected = false;
}

void Client::PrintIPAddresses()
{
    _networkManager.PrintAddressInfo();
}

std::string Client::GetUserMessage()
{
    std::string message;
    std::getline(std::cin, message);
    return message;
}
