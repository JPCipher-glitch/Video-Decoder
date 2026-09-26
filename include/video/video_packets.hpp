#ifndef VIDEO_PACKETS_HPP
#define VIDEO_PACKETS_HPP

#include <string>

struct IPacket
{
	std::string command;
	std::string name;
};

struct PacketLoad : public IPacket
{
	std::string path;

	std::string frameFormat = ".jpg";
	std::string audioFormat = ".ogg";

	uint16_t width = 1920u;
	uint16_t height = 1080u;
	float fps = 60.f;
};

#endif
