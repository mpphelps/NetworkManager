#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#ifdef NETWORKUTILITIES_EXPORTS
#define NETWORKUTILITIES_API __declspec(dllexport)
#else
#define NETWORKUTILITIES_API __declspec(dllimport)
#endif

#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string>

#include <iostream>
#include <map>
#include <vector>

#define BACKLOG 10
#define MAXDATASIZE 4208

class Connection
{
public:
    int Socket = -1;
    struct sockaddr_storage Address = {};

};

class NetworkManager
{
private:
    SOCKET _parentSocket = -1;
    struct addrinfo* _info{};
    Connection _connection;
    std::map<SOCKET, sockaddr_storage> _socketAddresses;
    std::map<SOCKET, int> _pollfdIndex;
    std::vector<pollfd> _pollfds;

public:
    NetworkManager();
    ~NetworkManager();

    void ResolveAddress(PCSTR address, PCSTR port);
    void PrintAddressInfo();
    void SetIpAddress(PCSTR address);
    char* GetIpAddress();

    bool CreateParentSocket();
    bool BindSocket();
    bool NextAddressInfo();
    bool ConnectToServer();
    bool StartListening();
    void ReapDeadConnections();
    bool AcceptConnection();
    void CloseConnection();
    void CloseConnection(SOCKET socket);
    void CloseAllConnections();
    void Send(std::string message);
    void Send(std::string message, SOCKET socket);
    std::string Receive();
    std::string Receive(SOCKET socket);
    void LogError(std::string errorMessage);
    std::string GetPeerName(SOCKET socket);
    bool PollIsParentReady(INT timeout);
    std::vector<SOCKET> PollChildren(INT timeout);
private:
    void AddToPollList(SOCKET* socket, sockaddr_storage* address = nullptr);
    void DeleteSocket(SOCKET* socket);
    void SendMessageSize(SOCKET socket, size_t bufferSize);
    size_t ReceiveMessageSize(SOCKET socket);
    void PollSockets(std::vector<pollfd>* fdArray, ULONG fds, INT timeout);
};

#endif // NETWORKMANAGER_H
