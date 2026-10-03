#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "asio_utils.hpp"
#include "serializer/protocol.hpp"
#include "serializer/command_type.hpp"
#include "observer.hpp"

#include <array>
#include <future>
#include <shared_mutex>
#include <thread>
#include <vector>

using byte = uint8_t;
using byte_stream = std::vector<uint8_t>;

class Command;

struct ClientStuff : public std::enable_shared_from_this<ClientStuff>
{
	TcpSocket socket;
	std::array<uint8_t, 1024> buffer;
	std::atomic_bool toRemove{ false };

	ClientStuff(TcpSocket&& socket);
	void read(std::weak_ptr<Command> command);
};

class Serializer;

class Command : public IProtocol, public std::enable_shared_from_this<Command>
{
private:
	IoContext ctx;
	TcpSocket server;
	Acceptor acceptor;

	std::vector<IObserver*> obsList;
	std::unordered_map<COMMAND_TYPE, std::function<std::unique_ptr<IPacket>(Serializer&)>> callMap;
	std::vector<std::shared_ptr<ClientStuff>> clientList;

	std::shared_mutex mutex;
	std::atomic_bool running = true;
	std::jthread task;
public:
	Command();
	~Command();

	bool isRunning() { return running; };

	std::unique_ptr<IPacket> loadVideo(Serializer& s);
	std::unique_ptr<IPacket> receivePacket(const byte_stream& stream);

	void attach(IObserver* obs);
	void notify(IPacket* packet);

	void accept();
	void update();
	void run();
};

#endif
