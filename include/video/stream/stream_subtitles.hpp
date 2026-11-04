#ifndef STREAM_SUBTITLES_HPP
#define STREAM_SUBTITLES_HPP

#include "video/stream/stream.hpp"

class StreamSubtitles : public IVidStream
{
private:
	bool invalid{ false };
public:
	StreamSubtitles(ErrorHandler& errorHandler);
	~StreamSubtitles() override;

	void loadDecoder() override;
	void loadEncoder() override;
	void loadFrame() override;
	void receiveFrame(AVFormatContext*& formatContext) override;
	void saveAsFile() override;
};

#endif
