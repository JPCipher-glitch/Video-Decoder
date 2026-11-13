#include "debug.hpp"

#include "video/video_packets.hpp"
#include "serializer/serializer.hpp"
#include "lifetime.hpp"
#include "error_handler.hpp"

#include <conio.h>
#include <iostream>
#include <sstream>
#include <iomanip>

#pragma region CONSTRUCTOR
Debug::Debug(Lifetime& lifetime, ErrorHandler& errorHandler) : lifetime(lifetime), errorHandler(errorHandler), socket{ctx}, resolver{ctx}
{
    // Fill the map
    callMap.emplace("LOAD", [this](Serializer& s, std::stringstream& stream) { loadVideo(s, stream); });

    // Fill the command map
    commandMap.emplace("--name", [this](std::stringstream& stream, IPacket* p)
        {
            stream >> std::quoted(p->name); 
            p->name = p->name.empty() ? "Dewey & Della - Singing" : p->name;
        });
    commandMap.emplace("--path", [this](std::stringstream& stream, IPacket* p)
        {
            PacketLoad* packet = dynamic_cast<PacketLoad*>(p);
            stream >> std::quoted(packet->path);
            packet->path = packet->path.empty() ? "placeholders/ducktales" : packet->path;
        });
    commandMap.emplace("--video-format", [this](std::stringstream& stream, IPacket* p)
        {
            PacketLoad* packet = dynamic_cast<PacketLoad*>(p);
            stream >> std::quoted(packet->videoFormat);
            packet->videoFormat = packet->videoFormat.empty() || packet->videoFormat.front() != '.' ? ".mp4" : packet->videoFormat;
        });
    commandMap.emplace("--audio-format", [this](std::stringstream& stream, IPacket* p)
        {
            PacketLoad* packet = dynamic_cast<PacketLoad*>(p);
            stream >> std::quoted(packet->audioFormat);
            packet->audioFormat = packet->audioFormat.empty() || packet->audioFormat.front() != '.' ? ".wav" : packet->audioFormat;
        });
    commandMap.emplace("--frame-format", [this](std::stringstream& stream, IPacket* p)
        {
            PacketLoad* packet = dynamic_cast<PacketLoad*>(p);
            stream >> std::quoted(packet->frameFormat);
            packet->frameFormat = packet->frameFormat.empty() || packet->frameFormat.front() != '.' ? ".jpg" : packet->frameFormat;
        });
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
void Debug::loadVideo(Serializer& s, std::stringstream& ss)
{
    PacketLoad packet{};
    packet.timePoint = std::chrono::duration<float>(std::chrono::steady_clock::now().time_since_epoch()).count(); // Get the last time point to display after the sending packet duration

    // Extract each commands
    std::string command;
    while (ss >> command)
    {
        if (commandMap.contains(command))
            commandMap[command](ss, &packet);
        else
            errorHandler.showWarning(std::format("This sub-command name is invalid: {}", command), false);
    }

    ss >> std::quoted(packet.name) >> std::quoted(packet.path);

    // Make sure the name & the path is not invalid
    packet.name = packet.name.empty() ? "Dewey & Della - Singing" : packet.name;
    packet.path = packet.path.empty() ? "placeholders/ducktales" : packet.path;

    s.write(COMMAND_TYPE::LOAD);
    s.write(packet.timePoint);

    s.writeString(packet.name);
    s.writeString(packet.path);
    s.writeString(packet.frameFormat);
    s.writeString(packet.audioFormat);
    s.writeString(packet.videoFormat);

    s.write(packet.width);
    s.write(packet.height);
    s.write(packet.fps);

    s.write(packet.audioSampleRate);
    s.write(packet.audioChannels);
    s.write(packet.audioQuality);
}

void Debug::sendPacket(const std::string& name, std::stringstream& ss)
{
    // Get the command name
    command = name;

    // Serialize the data content
    Serializer s{};
    callMap[command](s, ss);

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
            std::string input;
            std::getline(std::cin, input);

            std::stringstream ss(input);
            std::string command;

            ss >> command;

            // Check if the command is valid
            if (command == "QUIT")
                lifetime.destroy();
            else if (callMap.contains(command))
                sendPacket(command, ss);
            else
            {
                errorHandler.showWarning(std::format("This command name is invalid: {}", command), false);
                std::cerr << "INVALID COMMAND: " << command << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
#pragma endregion