#ifndef STREAM_VIDEO_HPP
#define STREAM_VIDEO_HPP

#include "video/stream/stream.hpp"
#include <filesystem>
#include <fstream>

extern "C"
{
	#include <libswscale/swscale.h>
	#include <libavfilter/avfilter.h>
	#include <libavfilter/buffersrc.h>
	#include <libavfilter/buffersink.h>
	#include <libavutil/opt.h>
}

struct FilterData
{
	AVFilterGraph* filterGraph{ nullptr };
	AVFilterContext* bufferSrcContext{ nullptr };
	AVFilterContext* bufferSinkContext{ nullptr };
};

struct StreamFrameSettings
{
	std::string path;
	std::string format;
	AVStream* stream;
	int streamIndex;

	uint16_t width;
	uint16_t height;
	uint8_t fps;
};

class StreamVideo : public IVidStream
{
private:
	uint8_t frameIndex{ 0U };
	uint8_t preloadingFrameLimit = { 10U };
	StreamFrameSettings settings;
	SwsContext* swsContext{ nullptr };
	AVFrame* exportFrame{ nullptr };

	AVFrame* filteredFrame{ nullptr };

	FilterData filterData;

	void addProgress();
	void createSubFile();
	void createFramesCache();
	void createExportFrame();
	bool loadFilter();
	bool writeFrame(AVFormatContext*& formatContext);
	std::filesystem::path getFilePath(const std::string& format);
public:
	StreamVideo(StreamFrameSettings settings, ErrorHandler& errorHandler, VideoLifetime& videoLifetime, LoadingHandler& loadingHandler);
	~StreamVideo() override;

	void loadDecoder() override;
	void loadEncoder() override;
	void loadFrame() override;
	void loadSwsContext();
	void receiveFrame(AVFormatContext*& formatContext) override;
	void saveAsFile() override;
	void closeFile() override;
};

#endif
