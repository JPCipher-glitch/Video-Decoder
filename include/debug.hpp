#ifndef DEBUG_HPP
#define DEBUG_HPP

#include "asio_utils.hpp"
#include "serializer/protocol.hpp"

#include <functional>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <sstream>

class Lifetime;
class Serializer;
class ErrorHandler;
class IPacket;

class Debug : public IProtocol
{
private:
	IoContext ctx;
	TcpSocket socket;
	Resolver resolver;

	Lifetime& lifetime;
	ErrorHandler& errorHandler;

	std::string command;

	void loadVideo(Serializer& s, std::stringstream& stream);
	void sendPacket(const std::string& name, std::stringstream& stream);

	std::unordered_map<std::string, std::function<void(Serializer&, std::stringstream&)>> callMap;
	std::unordered_map<std::string, std::function<void(std::stringstream&, IPacket*)>> commandMap;

	std::mutex mutex;
	std::jthread task;

	void handlerTask(std::stop_token stopToken);
public:
	Debug(Lifetime& lifetime, ErrorHandler& errorHandler);
	~Debug();

	void run();
};

#endif