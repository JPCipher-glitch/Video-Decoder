#ifndef VIDEO_STUFF_HPP
#define VIDEO_STUFF_HPP

#include <unordered_map>
#include <unordered_set>

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
	std::unordered_map<AVMediaType, AVStream*> videoStreamMap;
	std::unordered_set<AVMediaType> availableStreamType{ AVMediaType::AVMEDIA_TYPE_VIDEO, AVMediaType::AVMEDIA_TYPE_AUDIO, AVMediaType::AVMEDIA_TYPE_SUBTITLE };
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
