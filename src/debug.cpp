#include "debug.hpp"

#include "serializer/serializer.hpp"

#include <conio.h>
#include <iostream>
#include <sstream>

#pragma region COMMANDS
void Debug::sendPacket(const std::string& name)
{
    std::cout << "VIDEO_LOADED\n";
    //send(server, msg.c_str(), msg.length(), 0);
}
#pragma endregion



#pragma region CONSTRUCTOR
Debug::Debug()
{
    // Fill the map
    callMap["LOAD"] = [this]() { sendPacket(""); };

    WSAStartup(MAKEWORD(2, 2), &data); // Initialize Network Stack with a stable version of Winsock

    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // Create the socket of the server : AF_INET for IPV4, SOCK_STREAM & IPPROTO_TCP to use the TCP protocol

    sockaddr_in addr{}; // Create the Contact Form to 127.0.0.1 in the port 9666
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9666);

    if (connect(server, (sockaddr*)&addr, sizeof(addr)) < 0) // Check if the client succeed to connect to the Server, else destroy the client
    {
        std::cerr << "Error to connecting to the server.\n";
        return;
    }
    else // Send nickname to the server
    {
        std::cerr << "Connected to the server.\n";
    }

    // Create the task to manage the packet transfer
    task = std::jthread([this](std::stop_token stopToken)
    {
        handlerTask(stopToken);
    });
}
#pragma endregion

#pragma region DESTRUCTOR
Debug::~Debug()
{
    // Ask to the task to stop
    task.request_stop();

    // Destroy the server & the sockets when the task finishes
    if (server != INVALID_SOCKET)
    {
        shutdown(server, SD_BOTH);
        closesocket(server);
        server = INVALID_SOCKET;
    }
    WSACleanup();
}
#pragma endregion

#pragma region HANDLER
void Debug::handlerTask(std::stop_token stopToken)
{
    std::lock_guard lock(mutex); // Lock the others threads
    while (!stopToken.stop_requested()) // Execute while the server is still existing
    {
        // Check if a key has been pressed. Used to clean the task if you've not yet typing
        if (_kbhit())
        {
            std::string msg;
            std::getline(std::cin, msg);

            // Check if the command is valid
            if (callMap.contains(msg))
                callMap[msg]();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
#pragma endregion

#pragma region SERIALIZE
byte_stream Debug::serialize() const
{
    Serializer s{};
    /*s.write(GetType());
    s.write(grid_width);
    s.write(grid_height);*/

    return s.returnStream();
}
#pragma endregion

