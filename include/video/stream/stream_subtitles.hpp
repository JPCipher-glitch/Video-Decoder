#ifndef STREAM_SUBTITLES_HPP
#define STREAM_SUBTITLES_HPP

#include "video/stream/stream.hpp"
#include <filesystem>
#include <fstream>

struct StreamSubSettings
{
	std::string path;
	AVStream* stream;
	int streamIndex;
};

class StreamSubtitles : public IVidStream
{
private:
	bool invalid{ false };
	StreamSubSettings settings;
	std::ofstream outputFile;
public:
	StreamSubtitles(StreamSubSettings settings, ErrorHandler& errorHandler);
	~StreamSubtitles() override;

	void createSubFile();
	void createSubsCache();
	std::filesystem::path getFilePath(const std::string& format);
	std::string extractAssText(const char* ass);
	std::string extractText(const AVSubtitle& subtitle);
	std::string formatTimestamp(int64_t milliseconds);

	void loadDecoder() override;
	void loadEncoder() override;
	void loadFrame() override;
	void receiveFrame(AVFormatContext*& formatContext) override;
	void saveAsFile() override;
	void closeFile();
};

#endif
