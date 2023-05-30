#include <iostream>

#include "Server.h"


int main()
{
    Server server("3490");
    server.Initialize();

    while(server.InSession)
    {
        if (server.IsConnected)
        {
            std::vector<SOCKET> readySockets = server.PollConnections();
            for (SOCKET socket : readySockets)
            {
                std::string message = server.Receive(socket);
                std::cout << "Received message from the client: " << message << std::endl;
                if (message == "End Session")
                {
                    server.Send("Ending Server Session.", socket);
                    server.Close();
                    std::cout << "End of Session." << std::endl;
                    break;
                }
                else
                {
                    server.Send("Acknowledge.", socket);
                }
            }
        }
        else if(server.HasIncomingConnection())
        {
            server.AcceptConnection();
        }
        else
        {
            printf("Waiting for connection request....\n");
            Sleep(1000);
        }
    }
}
