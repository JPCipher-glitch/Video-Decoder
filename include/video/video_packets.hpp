#ifndef VIDEO_PACKETS_HPP
#define VIDEO_PACKETS_HPP

#include "serializer/command_type.hpp"
#include <string>

struct IPacket
{
	std::string name;
	COMMAND_TYPE type;
	double timePoint = 0;
	virtual ~IPacket() {};
};

struct PacketLoad : public IPacket
{
	std::string path;

	// File formats
	std::string frameFormat = ".jpg";
	std::string audioFormat = ".wav";
	std::string videoFormat = ".mp4";

	// Video settings
	uint16_t width = 1920U;
	uint16_t height = 1080U;
	uint8_t fps = 5U;

	// Audio settings
	uint32_t audioSampleRate = 44100U;
	uint16_t audioChannels = 2U;
	int8_t audioQuality = 6U; // Apply only for .ogg audio

	~PacketLoad() override {};
};

#endif
