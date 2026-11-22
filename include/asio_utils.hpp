#ifndef ASIO_UTILS_HPP
#define ASIO_UTILS_HPP

#include <asio.hpp>

// Server port used according to the type of instance executing
#ifdef _DEBUG
constexpr unsigned short SERVER_PORT = 6464;
#else
constexpr unsigned short SERVER_PORT = 6767;
#endif

// ASIO Common Aliases
using IoContext = asio::io_context;
using ExecutorWorkGuard = asio::executor_work_guard<IoContext::executor_type, void, void>;
using Resolver = asio::ip::tcp::resolver;
using ResultType = Resolver::results_type;

// TCP
using TcpSocket = asio::ip::tcp::socket;
using TcpEndpoint = asio::ip::tcp::endpoint;
using Acceptor = asio::ip::tcp::acceptor;

// UDP
using UdpSocket = asio::ip::udp::socket;
using UdpEndpoint = asio::ip::udp::endpoint;

// Common
using ErrorCode = std::error_code;

#endif