#ifndef STREAM_VIDEO_HPP
#define STREAM_VIDEO_HPP

#include "video/stream/stream.hpp"
#include <filesystem>
#include <fstream>

extern "C"
{
	#include <libswscale/swscale.h>
}

struct StreamFrameSettings
{
	std::string path;
	std::string format;
	AVStream* stream;
	int streamIndex;

	uint16_t width;
	uint16_t height;
	float fps;
};

class StreamVideo : public IVidStream
{
private:
	StreamFrameSettings settings;
	SwsContext* swsContext{ nullptr };

	void createSubFile();
	void createFramesCache();
	bool writeFrame(AVFormatContext*& formatContext);
	std::filesystem::path getFilePath(const std::string& format);
public:
	StreamVideo(StreamFrameSettings settings, ErrorHandler& errorHandler);
	~StreamVideo() override;

	void loadDecoder() override;
	void loadEncoder() override;
	void loadFrame() override;
	void loadSwsContext();
	void receiveFrame(AVFormatContext*& formatContext) override;
	void saveAsFile() override;
	void closeFile();
};

#endif
