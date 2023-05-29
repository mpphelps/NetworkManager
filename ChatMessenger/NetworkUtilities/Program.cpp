#include <iostream>
#include "Client.h"

int main()
{
    Client client("localhost", "3490");

    client.Initialize();
    client.PrintIPAddresses();

    client.Connect();
    Sleep(1000);

    client.Send("Hello World 1!");
    std::string message = client.Receive();
    std::cout << "Received message from the server: " << message << std::endl;

    client.Send("Hello World 2!");
    message = client.Receive();
    std::cout << "Received message from the server: " << message << std::endl;

    client.Send("Hello World 3!");
    message = client.Receive();
    std::cout << "Received message from the server: " << message << std::endl;

    client.Close();
}
