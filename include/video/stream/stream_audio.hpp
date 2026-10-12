#ifndef STREAM_AUDIO_HPP
#define STREAM_AUDIO_HPP

#include "audio/wav_writer.hpp"
#include "video/stream/stream.hpp"

extern "C"
{
	#include <libswresample/swresample.h>
}

class StreamAudio : public IStream
{
private:
	AVChannelLayout outputLayout = AV_CHANNEL_LAYOUT_STEREO;
	SwrContext* swrContext = nullptr;

	AVFrame* convertedFrame = nullptr;
	std::unique_ptr<WavWriter> wavWriter;
public:
	StreamAudio(std::string path, AVStream* stream, int streamIndex);
	~StreamAudio() override;

	void loadDecoder() override;
	void loadEncoder() override;
	void loadFrame() override;
	void loadSwrContext();
	void receiveFrame(AVFormatContext*& formatContext) override;
	void saveAsFile() override;
};

#endif
