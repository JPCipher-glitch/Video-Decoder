#ifndef VIDEO_STUFF_HPP
#define VIDEO_STUFF_HPP

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libavutil/avutil.h>
	#include <libswscale/swscale.h>
}

struct VideoStuff final
{
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
};

#endif
