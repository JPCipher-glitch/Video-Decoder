#ifndef VIDEO_PACKETS_HPP
#define VIDEO_PACKETS_HPP

#include "serializer/command_type.hpp"
#include <string>

struct IPacket
{
	std::string name;
	COMMAND_TYPE type;
	virtual ~IPacket() {};
};

struct PacketLoad : public IPacket
{
	std::string path;

	std::string frameFormat = ".jpg";
	std::string audioFormat = ".ogg";
	std::string videoFormat = ".mp4";

	uint16_t width = 1920u;
	uint16_t height = 1080u;
	float fps = 60.f;

	~PacketLoad() override {};
};

#endif
