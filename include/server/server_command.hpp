#ifndef SERVER_COMMAND_HPP
#define SERVER_COMMAND_HPP

#include "server/server_client.hpp"

#include "asio_utils.hpp"
#include "serializer/protocol.hpp"
#include "serializer/command_type.hpp"
#include "observer.hpp"
#include "error_handler.hpp"

#include <array>
#include <future>
#include <shared_mutex>
#include <thread>
#include <vector>

using byte = uint8_t;
using byte_stream = std::vector<uint8_t>;

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
	Command(ErrorHandler& errorHandler);
	~Command();

	ErrorHandler& errorHandler;
	std::queue<std::unique_ptr<IPacket>> packetQueue;
	std::mutex packetMutex;

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
