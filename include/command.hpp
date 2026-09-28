#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "serializer/protocol.hpp"
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

class Command : public IProtocol
{
private:
	NetworkStuff stuff;
	std::vector<IObserver*> obsList;

	std::vector<std::unique_ptr<ClientStuff>> clientList;

	std::shared_mutex mutex;
	std::future<void> task;
	std::atomic_bool running = true;

	void handlerTask();
public:
	Command();
	~Command();

	void attach(IObserver* obs);
	void notify(IPacket* packet);

	void update();

	byte_stream serialize() const override { return {}; };
	void deserialize(const byte_stream& data) override {};
};

#endif
