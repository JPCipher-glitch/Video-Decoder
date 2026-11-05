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
	AVSubtitle subtitle{};
	StreamSubSettings settings;
	std::ofstream outputFile;
	int gotSubtitle = 0;
	int subsIndex = 0;

	bool writeFrame(AVFormatContext*& formatContext);
	void createSubFile();
	void createSubsCache();
	std::filesystem::path getFilePath(const std::string& format);
	std::string extractAssText(const char* ass);
	std::string extractText();
	std::string formatTimestamp(int64_t milliseconds);
public:
	StreamSubtitles(StreamSubSettings settings, ErrorHandler& errorHandler);
	~StreamSubtitles() override;

	void loadDecoder() override;
	void loadEncoder() override;
	void loadFrame() override;
	void receiveFrame(AVFormatContext*& formatContext) override;
	void saveAsFile() override;
	void closeFile();
};

#endif
