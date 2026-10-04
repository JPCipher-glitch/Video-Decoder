#ifndef DEBUG_HPP
#define DEBUG_HPP

#include "asio_utils.hpp"
#include "serializer/protocol.hpp"

#include <functional>
#include <thread>
#include <mutex>
#include <unordered_map>

class Lifetime;
class Serializer;

class Debug : public IProtocol
{
private:
	IoContext ctx;
	TcpSocket socket;
	Resolver resolver;

	Lifetime& lifetime;

	std::string command;

	void loadVideo(Serializer& s);
	void sendPacket(const std::string& name);

	std::unordered_map<std::string, std::function<void(Serializer&)>> callMap;

	std::mutex mutex;
	std::jthread task;

	void handlerTask(std::stop_token stopToken);
public:
	Debug(Lifetime& lifetime);
	~Debug();

	void run();
};

#endif
