#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <vector>

using byte = uint8_t;
using byte_stream = std::vector<uint8_t>;

class IProtocol
{
protected:
	byte_stream data;
public:
};

#endif