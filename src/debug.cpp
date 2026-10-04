#include "debug.hpp"

#include "video/video_packets.hpp"
#include "serializer/serializer.hpp"
#include "lifetime.hpp"

#include <conio.h>
#include <iostream>
#include <sstream>

#pragma region CONSTRUCTOR
Debug::Debug(Lifetime& lifetime) : lifetime(lifetime), socket{ ctx }, resolver{ ctx }
{
    // Fill the map
    callMap["LOAD"] = [this](Serializer& s) { loadVideo(s); };
}
#pragma endregion

#pragma region DESTRUCTOR
Debug::~Debug()
{
    // Ask to the task to stop
    task.request_stop();
}
#pragma endregion

#pragma region RUN
void Debug::run()
{
    // Create the connection between the debug client & the server
    ResultType endpoints = resolver.resolve("127.0.0.1", std::to_string(SERVER_PORT));
    asio::connect(socket, endpoints);

    // Create the task to manage the packet transfer
    task = std::jthread([this](std::stop_token stopToken)
    {
        handlerTask(stopToken);
    });

    std::cerr << "Connected to the server.\n";
}
#pragma endregion


#pragma region COMMANDS
void Debug::loadVideo(Serializer& s)
{
    PacketLoad packet{};
    /*packet.name = "Bart at the Blarney Stone";
    packet.path = "C:/Tailscoco/Video/placeholder";*/

    packet.name = "Dewey & Della - Singing";
    packet.path = "resources/ducktales";

    s.write(COMMAND_TYPE::LOAD);

    s.writeString(packet.name);
    s.writeString(packet.path);
    s.writeString(packet.frameFormat);
    s.writeString(packet.audioFormat);
    s.writeString(packet.videoFormat);

    s.write(packet.width);
    s.write(packet.height);
    s.write(packet.fps);
}

void Debug::sendPacket(const std::string& name)
{
    // Get the command name
    command = name;

    // Serialize the data content
    Serializer s{};
    callMap[command](s);

    // Send the new stream data to the server
    byte_stream stream = s.returnStream();
    size_t bytes = asio::write(socket, asio::buffer(stream));
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
            if (msg == "QUIT")
                lifetime.destroy();
            else if (callMap.contains(msg))
                sendPacket(msg);
            else
                std::cerr << "INVALID COMMAND: " << msg << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
#pragma endregion

