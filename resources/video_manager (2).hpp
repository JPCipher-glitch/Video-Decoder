#ifndef VIDEO_MANAGER_HPP
#define VIDEO_MANAGER_HPP

#include "lifetime.hpp"

#include <string>

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libavutil/avutil.h>
	#include <libswscale/swscale.h>
}

class VideoManager
{
private:
	// Lifetime
	Lifetime* lifetime = nullptr;

	// Video Settings
	const std::string path = "resources";
	const std::string name = "ducktales";
	const std::string ext = "mp4";

	// Video Main Context
	AVFormatContext* formatContext = nullptr;

	// Video stream index
	int videoStreamIndex = -1;

	// Video informations
	AVStream* videoStream = nullptr;
	AVCodecParameters* codecParameters = nullptr;

	// Video decoder
	const AVCodec* codec = nullptr;

	// Decoder context
	AVCodecContext* codecContext = nullptr;

	// Frame packet
	AVPacket* packet = nullptr;

	// Context to converter
	SwsContext* swsContext = nullptr;
	// Frames Decoded and RGB
	AVFrame* decodedFrame = nullptr;
	AVFrame* rgbFrame = nullptr;

	// Frame Count
	int frameCount = 0;

	// Creation Methods
	void removeCache();
	void openVideo();
	void displayInfo();
	void createPacket();
public:
	VideoManager(Lifetime* lifetime);
	~VideoManager();

	void update();
};

#endif
