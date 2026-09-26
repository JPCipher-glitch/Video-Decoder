#ifndef COMMAND_HPP
#define COMMAND_HPP

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

class Command
{
private:
	NetworkStuff stuff;
	std::vector<IObserver*> obsList;

	std::vector<std::unique_ptr<ClientStuff>> clientList;

	std::shared_mutex mutex;
	std::condition_variable cv;
	std::future<void> task;
	std::atomic_bool running = true;

	void handlerTask();
public:
	Command();
	~Command();

	void attach(IObserver* obs);
	void notify(IPacket* packet);

	void update();
};

#endif
