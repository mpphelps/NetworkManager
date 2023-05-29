#include <iostream>

#include "Server.h"


int main()
{
    Server server("3490");
    server.Initialize();
    server.WaitForConnection();
    int i = 0;
    while (i++ <= 3)
    {
        std::vector<SOCKET> readySockets = server.PollConnections();
        for (SOCKET socket : readySockets)
        {
            std::string message = server.Receive(socket);
            std::cout << "Received message from the client: " << message << std::endl;
            server.Send("Received messaged.", socket);
        }
    }
    
    server.Close();
}
