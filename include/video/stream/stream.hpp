#ifndef STREAM_HPP
#define STREAM_HPP

#include <string>
extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libavutil/avutil.h>
}

class IStream
{
protected:
	// Path
	std::string path;
	// Stream
	AVStream* stream = nullptr;
	int streamIndex = -1;

	// Decoder
	const AVCodec* decoder = nullptr;
	AVCodecContext* decoderContext = nullptr; // Context
	// Encoder
	const AVCodec* encoder = nullptr;
	AVCodecContext* encoderContext = nullptr; // Context

	// Frame packet
	AVPacket* sendPacket = nullptr;
	AVPacket* receivePacket = nullptr;
	AVFrame* frame = nullptr;
public:
	IStream(std::string path, AVStream* stream, int streamIndex) : path{ path }, stream { stream }, streamIndex{ streamIndex } {};
	virtual ~IStream() {};
	virtual void loadDecoder() = 0;
	virtual void loadEncoder() = 0;
	virtual void loadFrame() = 0;
	virtual void receiveFrame(AVFormatContext*& formatContext) = 0;
	virtual void saveAsFile() = 0;
};

#endif
