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
	std::string audioFormat = ".wav";
	std::string videoFormat = ".mp4";

	uint16_t width = 1920u;
	uint16_t height = 1080u;
	float fps = 60.f;

	uint32_t audioSampleRate = 44100u;
	uint16_t audioChannels = 2u;
	int8_t audioQuality = 6u; // Apply only for .ogg audio

	~PacketLoad() override {};
};

#endif
