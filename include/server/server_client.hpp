#ifndef SERVER_CLIENT_HPP
#define SERVER_CLIENT_HPP

#include "asio_utils.hpp"
#include <array>

class Command;
struct ClientStuff : public std::enable_shared_from_this<ClientStuff>
{
	TcpSocket socket;
	std::array<uint8_t, 1024> buffer;
	std::atomic_bool toRemove{ false };

	ClientStuff(TcpSocket&& socket);
	void read(std::weak_ptr<Command> command);
};
#endif
