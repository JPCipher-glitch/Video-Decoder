#include "command.hpp"
#include "video/video_packets.hpp"

#include <iostream>

#pragma region CONSTRUCTOR
Command::Command()
{
    WSAStartup(MAKEWORD(2, 2), &stuff.data); // Initialize Network Stack with a stable version of Winsock

    stuff.server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // Create the socket of the server : AF_INET for IPV4, SOCK_STREAM & IPPROTO_TCP to use the TCP protocol
    u_long mode = 1;
    ioctlsocket(stuff.server, FIONBIO, &mode); // Set the socket to non-blocking

    sockaddr_in addr{}; // Create the Contact Form to 127.0.0.1 in the port 9666
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9666);

    bind(stuff.server, (sockaddr*)&addr, sizeof(addr)); // Assign the Network Adress of the server socket
    listen(stuff.server, 5); // Receive connection requests from others clients : 1 only is allowed

    std::cout << "Server is listening...\n";

    // Create the task to manage the packet transfer
    task = std::async(std::launch::async, [this]
    {
        handlerTask();
    });
}
#pragma endregion

#pragma region DESTRUCTOR
Command::~Command()
{
    running = false;

    // Wait the task to finish
    if (task.valid())
        task.wait();

    // Destroy the server & the sockets when the task finishes
    for (auto& client : clientList)
    {
        closesocket(client->socket);
    }
    closesocket(stuff.server);
    WSACleanup();
}
#pragma endregion

#pragma region HANDLER
void Command::handlerTask()
{
    while (running) // Execute while the server is still existing
    {
        //std::shared_lock lock(mutex); // Lock the readers to prevent of using the sockets when adding another one

        // Get the packets from each sockets
        for (auto client = clientList.begin(); client != clientList.end();)
        {
            // Create the packet to receive
            IPacket packet;

            // Take the sended bytes for the packet & check if the socket is still connected
            int result = recv((*client)->socket, reinterpret_cast<char*>(&packet), sizeof(packet), 0);
            if (result > 0) // The client socket is still connected
            {
                // ...
                notify(&packet);
                client++;
            }
            else if (result == 0) // The client socket is cleanly deleted
            {
                {
                    std::unique_lock lock(mutex);
                    closesocket((*client)->socket);
                    client = clientList.erase(client);
                }

                std::cout << "Client disconnected.\n";
            }
            else if (result == SOCKET_ERROR) // An error was ocurred in the socket
            {
                int error = WSAGetLastError();
                if (error == WSAEWOULDBLOCK)
                {
                    client++;
                }
                else
                {
                    {
                        std::unique_lock lock(mutex);
                        closesocket((*client)->socket);
                        client = clientList.erase(client);
                    }

                    std::cout << "ERROR: Client suddenly disconnected.\n";
                }
            }
        }
    }
}
#pragma endregion

#pragma region UPDATE
void Command::update()
{
    SOCKET socket = accept(stuff.server, nullptr, nullptr); // Accept client request if received
    if (socket != INVALID_SOCKET) // Check if request received
    {
        // Set the socket to non-blocking
        u_long mode = 1;
        ioctlsocket(socket, FIONBIO, &mode);

        // Create the client
        std::unique_ptr<ClientStuff> client = std::make_unique<ClientStuff>();
        client->socket = socket;

        // Add the client to the list
        {
            std::unique_lock lock(mutex); // Lock the readers to prevent of using the sockets when adding another one
            clientList.push_back(std::move(client));
        }
    }
}
#pragma endregion

#pragma region OBSERVER
void Command::attach(IObserver* obs)
{
    obsList.push_back(obs);
}

void Command::notify(IPacket* packet)
{
    for (auto& obs: obsList)
    {
        obs->call(packet);
    }
}
#pragma endregion
