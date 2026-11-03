#ifndef STREAM_HPP
#define STREAM_HPP

#include "error_handler.hpp"
#include <string>
extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libavutil/avutil.h>
}

class IVidStream
{
protected:
	// Path
	std::string path;
	// Stream
	AVStream* stream = nullptr;
	int streamIndex = -1;

	// Error Handler
	ErrorHandler& errorHandler;

	// Decoder
	const AVCodec* decoder = nullptr;
	AVCodecContext* decoderContext = nullptr; // Context
	// Encoder
	const AVCodec* encoder = nullptr;
	AVCodecContext* encoderContext = nullptr; // Context
	AVFormatContext* formatContext = nullptr;

	// Frame packet
	AVPacket* sendPacket = nullptr;
	AVPacket* receivePacket = nullptr;
	AVFrame* frame = nullptr;
public:
	IVidStream(ErrorHandler& errorHandler, std::string path, AVStream* stream, int streamIndex) : errorHandler{ errorHandler }, path { path }, stream{ stream }, streamIndex{ streamIndex } {};
	virtual ~IVidStream() {};
	virtual void loadDecoder() = 0;
	virtual void loadEncoder() = 0;
	virtual void loadFrame() = 0;
	virtual void receiveFrame(AVFormatContext*& formatContext) = 0;
	virtual void saveAsFile() = 0;
};

#endif
