#ifndef STREAM_AUDIO_HPP
#define STREAM_AUDIO_HPP

#include "audio/wav_writer.hpp"
#include "video/stream/stream.hpp"
#include <vector>

extern "C"
{
	#include <libswresample/swresample.h>
}

class StreamAudio : public IVidStream
{
private:
	int outputLayoutIndex = 0;
	std::vector<AVChannelLayout> outputLayoutList{ AV_CHANNEL_LAYOUT_MONO, AV_CHANNEL_LAYOUT_STEREO };
	SwrContext* swrContext = nullptr;

	AVFrame* convertedFrame = nullptr;
	std::unique_ptr<WavWriter> wavWriter;

	uint32_t audioSampleRate; 
	uint16_t audioChannels;
public:
	StreamAudio(std::string path, AVStream* stream, int streamIndex);
	~StreamAudio() override;

	void createAudioWriter(uint32_t audioSampleRate, uint16_t audioChannels);

	void loadDecoder() override;
	void loadEncoder() override;
	void loadFrame() override;
	void loadSwrContext();
	void loadFormatContext();
	void receiveFrame(AVFormatContext*& formatContext) override;
	void saveAsFile() override;
};

#endif
