#ifndef DEBUG_HPP
#define DEBUG_HPP

#include "serializer/protocol.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>
#define NOMINMAX
#include <windows.h>

#include <functional>
#include <thread>
#include <mutex>
#include <unordered_map>

class Serializer;

class Debug : public IProtocol
{
private:
	WSADATA data;
	SOCKET server;

	std::string command;

	void loadVideo(Serializer& s);
	void sendPacket(const std::string& name);

	std::unordered_map<std::string, std::function<void(Serializer&)>> callMap;

	std::mutex mutex;
	std::jthread task;

	void handlerTask(std::stop_token stopToken);
public:
	Debug();
	~Debug();
};

#endif
