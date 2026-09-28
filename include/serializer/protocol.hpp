#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <vector>

enum class COMMAND_TYPE : uint8_t 
{
    LOAD,
};

using byte = uint8_t;
using byte_stream = std::vector<uint8_t>;

class IProtocol
{
protected:
	byte_stream data;
public:
	virtual byte_stream serialize() const = 0;
	virtual void deserialize(const byte_stream& data) = 0;
};

#endif