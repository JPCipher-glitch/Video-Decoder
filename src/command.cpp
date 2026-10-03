#include "command.hpp"
#include "video/video_packets.hpp"
#include "serializer/serializer.hpp"

#include <iostream>

#pragma region READ
ClientStuff::ClientStuff(TcpSocket&& socket) : socket(std::move(socket)) {}

void ClientStuff::read(std::weak_ptr<Command> c)
{
    auto self{ shared_from_this() }; // Make sure the client is not unvalid when updating the async task
    socket.async_read_some(asio::buffer(buffer), [this, self, c](ErrorCode error, size_t length)
        {
            std::shared_ptr<Command> command = c.lock();
            if (!command) return;
            if (!command->isRunning()) return;

            // Check when the client receive a packet
            if (!error && length > 0)
            {
                // Fill the stream byte with the buffer received
                byte_stream stream(buffer.begin(), buffer.begin() + length);

                if (command->isRunning())
                {
                    // Create the packet and send it to the video manager
                    std::unique_ptr<IPacket> packet = command->receivePacket(stream);
                    command->notify(packet.get());
                }

                read(command);
            }
            else if (error) // Check if an error occured
            {
                if (error != asio::error::eof)
                    std::cout << "ERROR: " << error.message() << std::endl;
                else
                    toRemove = true;
            }
        });
}
#pragma endregion

#pragma region CONSTRUCTOR
Command::Command() : server{ ctx }, acceptor{ ctx, TcpEndpoint(asio::ip::tcp::v4(), SERVER_PORT) }
{
    // Fill the map
    callMap[COMMAND_TYPE::LOAD] = [this](Serializer& s) { return loadVideo(s); };
    std::cout << "Server is listening...\n";
}
#pragma endregion

#pragma region DESTRUCTOR
Command::~Command()
{
    running = false;
    acceptor.close();
    ctx.stop();
}
#pragma endregion

#pragma region ACCEPT
void Command::accept()
{
    auto self = shared_from_this();
    acceptor.async_accept([this, self](ErrorCode error, TcpSocket socket)
        {
            if (!running) return;

            // Add the new client if there is no error during the process
            if (!error)
            {
                // Create the client
                std::shared_ptr<ClientStuff> client = std::make_shared<ClientStuff>(std::move(socket));
                client->read(self);

                // Add the client to the list
                {
                    std::unique_lock lock(mutex); // Lock the readers to prevent of using the sockets when adding another one
                    clientList.push_back(std::move(client));
                }
            }

            // Retry if there is other clients
            accept();
        });
}
#pragma endregion

#pragma region RUN
void Command::run()
{
    task = std::jthread([this]
    {
        ctx.run(); // Run the task launched with the context
    });
}
#pragma endregion

#pragma region COMMANDS
std::unique_ptr<IPacket> Command::loadVideo(Serializer& s)
{
    // Make the abstract packet and set it to the wanted packet
    std::unique_ptr<IPacket> packet = std::make_unique<PacketLoad>();
    PacketLoad* p = dynamic_cast<PacketLoad*>(packet.get());

    // Add the type of the packet
    p->type = COMMAND_TYPE::LOAD;

    // Read the string buffer
    p->name = s.readString();
    p->path = s.readString();
    p->frameFormat = s.readString();
    p->audioFormat = s.readString();
    p->videoFormat = s.readString();

    // Read the numerical data
    p->width = s.read<uint16_t>();
    p->height = s.read<uint16_t>();
    p->fps = s.read<float>();

    // Return the packet
    return std::move(packet);
}

std::unique_ptr<IPacket> Command::receivePacket(const byte_stream& stream)
{
    // Deserialize the first argument "TYPE" to determine with what type of packet we want to get
    Serializer s{ stream };
    COMMAND_TYPE type = s.read<COMMAND_TYPE>();

    // Return the packet
    return callMap[type](s);
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

#pragma region UPDATE
void Command::update()
{
    // Remove the clients when they are deconnected
    std::unique_lock lock(mutex);
    for (auto client = clientList.begin(); client != clientList.end();)
    {
        if ((*client)->toRemove)
        {
            client = clientList.erase(client); 
            continue;
        }
        client++;
    }
}
#pragma endregion


