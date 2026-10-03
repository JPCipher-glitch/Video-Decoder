#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "serializer/protocol.hpp"
#include "serializer/command_type.hpp"
#include "observer.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>
#define NOMINMAX
#include <windows.h>

#include <future>
#include <shared_mutex>
#include <vector>

struct ClientStuff
{
	SOCKET socket;
};

struct NetworkStuff
{
	WSADATA data;
	SOCKET server;
};

class Serializer;

class Command : public IProtocol
{
private:
	NetworkStuff stuff;
	std::vector<IObserver*> obsList;

	std::unordered_map<COMMAND_TYPE, std::function<std::unique_ptr<IPacket>(Serializer&)>> callMap;

	std::vector<std::unique_ptr<ClientStuff>> clientList;

	std::shared_mutex mutex;
	std::future<void> task;
	std::atomic_bool running = true;

	std::unique_ptr<IPacket> loadVideo(Serializer& s);
	std::unique_ptr<IPacket> receivePacket(const byte_stream& stream);

	void handlerTask();
public:
	Command();
	~Command();

	void attach(IObserver* obs);
	void notify(IPacket* packet);

	void update();
};

#endif
