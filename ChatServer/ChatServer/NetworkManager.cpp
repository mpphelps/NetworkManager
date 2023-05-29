#include "NetworkManager.h"

NetworkManager::NetworkManager()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(2, 0);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        printf("Error with WSA Startup.\n");
    }
}

NetworkManager::~NetworkManager()
{
    freeaddrinfo(_info);
}

void NetworkManager::ResolveAddress(PCSTR address, PCSTR port)
{
    addrinfo hints = {};
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status;
    if ((status = getaddrinfo(address, port, &hints, &_info)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }
}

void NetworkManager::PrintAddressInfo()
{
    char ipstr[INET6_ADDRSTRLEN];

    for (addrinfo* p = _info; p != nullptr; p = p->ai_next)
    {
        void* addr;
        const char* ipver;

        if (p->ai_family == AF_INET)
        {
            struct sockaddr_in* ipv4 = reinterpret_cast<struct sockaddr_in*>(p->ai_addr);
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        }
        else { // IPv6
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        // convert the IP to a string and print it:
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf("  %s: %s\n", ipver, ipstr);
    }
}

void NetworkManager::SetIpAddress(PCSTR address)
{
    ADDRESS_FAMILY family = _info->ai_addr->sa_family;
    if (family == AF_INET6)
    {
        sockaddr_in6* sa6 = reinterpret_cast<struct sockaddr_in6*>(_info->ai_addr);
        inet_pton(family, address, &sa6->sin6_addr);
    }
    else
    {
        sockaddr_in* sa = reinterpret_cast<struct sockaddr_in*>(_info->ai_addr);
        inet_pton(family, address, &sa->sin_addr);
    }
}

char* NetworkManager::GetIpAddress()
{
    ADDRESS_FAMILY family = _info->ai_addr->sa_family;
    if (family == AF_INET6)
    {
        char ip6[INET6_ADDRSTRLEN];
        sockaddr_in6* sa6 = reinterpret_cast<struct sockaddr_in6*>(_info->ai_addr);
        inet_ntop(AF_INET6, &sa6->sin6_addr, ip6, INET6_ADDRSTRLEN);
        return ip6;
    }
    else
    {
        char ip[INET_ADDRSTRLEN];
        sockaddr_in* sa = reinterpret_cast<struct sockaddr_in*>(_info->ai_addr);
        inet_ntop(AF_INET, &sa->sin_addr, ip, INET_ADDRSTRLEN);
        return ip;
    }
}

bool NetworkManager::CreateParentSocket()
{
    if (_info == NULL)
    {
        printf("Address is null.\n");
        return false;
    }

    while (_info != NULL)
    {
        if ((_parentSocket = socket(_info->ai_family, _info->ai_socktype, 0)) == -1)
        {
            LogError("Error creating socket file descriptor");
            _info = _info->ai_next;
        }
        AddToPollList(&_parentSocket);
        break;
    }

    if (_info == NULL)
    {
        printf("Failed to create socket.\n");
        return false;
    }

    printf("Successfully created a socket.\n");
    return true;

}

// Used by Servers
bool NetworkManager::BindSocket()
{
    if (_info == NULL)
    {
        printf("Address is null.\n");
        return false;;
    }

    int optVal = -1;
    int optLen = sizeof(int);
    if (setsockopt(_parentSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&optVal, optLen) == -1)
    {
        LogError("Error setting socket for reuse");
        exit(1);
    }

    if (bind(_parentSocket, _info->ai_addr, _info->ai_addrlen) == -1)
    {
        LogError("Error binding socket");
        closesocket(_parentSocket);
        return false;;
    }

    printf("Socket successfully bound to port.\n");
    return true;
}

bool NetworkManager::NextAddressInfo()
{
    if (_info == NULL)
    {
        printf("Address is null.\n");
        return false;
    }

    if (_info->ai_next == NULL)
    {
        printf("No more address in list.\n");
        return false;
    }

    _info = _info->ai_next;
    return true;
}

// Used by Clients
bool NetworkManager::ConnectToServer()
{
    int status = connect(_parentSocket, _info->ai_addr, _info->ai_addrlen);
    if (status == -1)
    {
        LogError("Error connecting socket");
        closesocket(_parentSocket);
        return false;
    }
    printf("Socket successfully connected.\n");
    return true;
}

// Used by Servers
bool NetworkManager::StartListening()
{
    if (listen(_parentSocket, BACKLOG) == -1)
    {
        LogError("Error listening on socket");
        return false;
    }
    else
    {
        struct sockaddr_storage addr;
        socklen_t len = sizeof(addr);

        if (getsockname(_parentSocket, (struct sockaddr*)&addr, &len) == -1)
        {
            LogError("Error retrieving socket information");
            return false;
        }

        if (addr.ss_family == AF_INET)
        {
            struct sockaddr_in* s = reinterpret_cast<struct sockaddr_in*>(&addr);
            printf("Listening on port# %d.\n", ntohs(s->sin_port));
        }
        else if (addr.ss_family == AF_INET6)
        {
            struct sockaddr_in6* s = reinterpret_cast<struct sockaddr_in6*>(&addr);
            printf("Listening on port# %d.\n", ntohs(s->sin6_port));
        }

        return true;
    }
}

// Used by Servers
bool NetworkManager::AcceptConnection()
{
    struct sockaddr_storage theirAddress;
    socklen_t sin_size = sizeof theirAddress;
    SOCKET childSocket = -1;
    while (childSocket == -1)
    {
        childSocket = accept(_parentSocket, reinterpret_cast<struct sockaddr*>(&theirAddress), &sin_size);
        if (childSocket == -1)
        {
            LogError("Error accepting request");
            return false;
        }
    }
    AddToPollList(&childSocket, &theirAddress);

    std::string peerName = GetPeerName(childSocket);
    printf("Connection accepted from %s.\n", peerName.c_str());

    return true;
}

void NetworkManager::CloseConnection()
{
    CloseConnection(_parentSocket);
}

void NetworkManager::CloseConnection(SOCKET socket)
{
    int status = shutdown(socket, SD_BOTH);
    if (status == -1)
    {
        LogError("Error shutting down socket connection");
    }
    status = closesocket(socket);
    if (status == -1)
    {
        LogError("Error closing socket connection");
    }
    DeleteSocket(&socket);
}

void NetworkManager::CloseAllConnections()
{
    for (pollfd poll : _pollfds)
    {
        if (poll.fd == _parentSocket) continue;
        printf("Closing connection to socket %llu.\n", poll.fd);
        int status = shutdown(poll.fd, SD_BOTH);
        if (status == -1)
        {
            LogError("Error shutting down socket connection");
        }
        status = closesocket(poll.fd);
        if (status == -1)
        {
            LogError("Error closing socket connection");
        }
        DeleteSocket(&poll.fd);
    }
}

void NetworkManager::Send(std::string message)
{
    Send(message, _parentSocket);
}

void NetworkManager::Send(std::string message, SOCKET socket)
{
    const char* buffer = message.c_str();
    size_t bufferSize = message.length();
    
    // First send the message size to peer
    SendMessageSize(socket, bufferSize);

    // Second send the message to peer
    size_t bytesSent;
    size_t total = 0;        // how many bytes we've sent
    size_t bytesleft = bufferSize; // how many we have left to send
    while (total < bufferSize)
    {
        if((bytesSent = send(socket, buffer + total, bytesleft, 0)) == -1)
        {
            LogError("Error sending message");
            break;
        }
        else
        {
            total += bytesSent;
            printf("%zu bytes sent for %zu of %zu\n", bytesSent, total, bufferSize);
            bytesleft -= bytesSent;
        }
    }
    printf("\n");
}

void NetworkManager::SendMessageSize(SOCKET socket, size_t bufferSize) 
{
    size_t networkBufferSize = htonl(bufferSize);
    char* messageSize = reinterpret_cast<char*>(&networkBufferSize);
    size_t bytesSent;

    bytesSent = send(socket, messageSize, sizeof size_t, 0);
    if (bytesSent == -1)
    {
        LogError("Error sending message length");
    }
    else if (bytesSent != sizeof size_t)
    {
        LogError("Error, incomplete message length sent");
    }
    else
    {
        printf("Told peer to receive %zu bytes.\n", bufferSize);
    }
}

std::string NetworkManager::Receive()
{
    return Receive(_parentSocket);
}

std::string NetworkManager::Receive(SOCKET socket)
{
    size_t bufferSize = ReceiveMessageSize(socket);
    char* buffer = new char[bufferSize + 1];

    size_t bytesReceived;
    size_t total = 0;
    size_t bytesleft = bufferSize;
    while (total < bufferSize)
    {
        if ((bytesReceived = recv(socket, buffer + total, bytesleft, 0)) == -1)
        {
            LogError("Error receiving message");
        }
        else 
        {
            total += bytesReceived;
            printf("%zu bytes received for %zu of %zu\n", bytesReceived, total, bufferSize);
            bytesleft -= bytesReceived;
        }
    }
    printf("\n");
    buffer[bufferSize] = '\0';
    std::string message(buffer);
    delete[] buffer;
    return message;
}

size_t NetworkManager::ReceiveMessageSize(SOCKET socket)
{
    size_t bytesReceived;
    size_t networkBufferSize;
    bytesReceived = recv(socket, reinterpret_cast<char*>(&networkBufferSize), sizeof size_t, 0);
    size_t bufferSize = ntohl(networkBufferSize);
    if (bytesReceived == -1)
    {
        LogError("Error receiving message length");
    }
    else if (bytesReceived != sizeof size_t)
    {
        LogError("Error, incomplete message length received");
    }
    else
    {
        printf("Told by peer to receive %zu bytes.\n", bufferSize);
    }
    return bufferSize;
}

void NetworkManager::LogError(std::string errorMessage)
{
    wchar_t* s = NULL;
    auto errorCode = WSAGetLastError();
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&s, 0, NULL);
    std::cerr << errorMessage << ". Error code: " << errorCode << std::endl;
    fprintf(stderr, "%S\n", s);
    WSACleanup();
}

std::string NetworkManager::GetPeerName(SOCKET socket)
{
    socklen_t len;
    struct sockaddr_storage addr;
    char ipstr[INET6_ADDRSTRLEN];
    int port;

    len = sizeof addr;
    if(getpeername(socket, (struct sockaddr*)&addr, &len) == -1)
    {
        LogError("Error getting peer name");
    }

    // deal with both IPv4 and IPv6:
    if (addr.ss_family == AF_INET) {
        struct sockaddr_in* s = (struct sockaddr_in*)&addr;
        port = ntohs(s->sin_port);
        inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
    }
    else { // AF_INET6
        struct sockaddr_in6* s = (struct sockaddr_in6*)&addr;
        port = ntohs(s->sin6_port);
        inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
    }

    /*printf("Peer IP address: %s\n", ipstr);
    printf("Peer port      : %d\n", port);*/
    std::string peerName;
    peerName.append("Address: ");
    peerName.append(ipstr);
    peerName.append(" Port#: ");
    peerName.append(std::to_string(port));
    return peerName;
}

std::vector<SOCKET> NetworkManager::PollSockets()
{
    std::vector<SOCKET> readySockets;
    printf("Polling sockets for data.\n");
    int num_events = WSAPoll(_pollfds.data(), _pollfds.size(), 10000);
    if (num_events == 0)
    {
        printf("Poll timed out!\n");
    }
    else if(num_events == -1)
    {
        LogError("Error polling sockets");
    }
    else
    {
        for (auto& pollfd : _pollfds)
        {
            int pollInHappened = pollfd.revents & POLLIN;
            if (pollInHappened && pollfd.fd == _parentSocket)
            {
                printf("Parent socket is ready to read");
                AcceptConnection();
            }
            else if (pollInHappened)
            {
                printf("File descriptor %llu is ready to read.\n", pollfd.fd);
                if (pollfd.fd == _parentSocket) continue;
                else
                {
                    readySockets.push_back(pollfd.fd);
                }
            }
            else
            {
                printf("File descriptor %llu has no events queued.\n", pollfd.fd);
            }
        }
    }
    return readySockets;
}

void NetworkManager::AddToPollList(SOCKET* socket, sockaddr_storage* theirAddress)
{
    pollfd newPoll{};
    newPoll.fd = *socket;
    newPoll.events = POLLIN;
    _pollfds.push_back(newPoll);
    if(theirAddress != nullptr)
        _socketAddresses[*socket] = *theirAddress;
    _pollfdIndex[*socket] = _pollfds.size() - 1;
}

void NetworkManager::DeleteSocket(SOCKET* socket)
{
    auto it = _pollfds.begin() + _pollfdIndex[*socket];
    _pollfds.erase(it);
    _socketAddresses.erase(*socket);
    _pollfdIndex.erase(*socket);
}
