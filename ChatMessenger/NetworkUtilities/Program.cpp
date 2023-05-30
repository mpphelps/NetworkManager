#include <iostream>
#include "Client.h"

int main()
{
    Client client("localhost", "3490");

    client.Initialize();
    client.PrintIPAddresses();

    client.Connect();
    Sleep(1000);

    while(client.IsConnected)
    {
        std::string message = client.GetUserMessage();
        client.Send(message);
        std::string returnMessage = client.Receive();
        std::cout << "Received message from the server: " << returnMessage << std::endl;
        if (message == "End Session")
        {
            client.Close();
        }
    }

    
}
