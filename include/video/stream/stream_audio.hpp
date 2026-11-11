#ifndef STREAM_AUDIO_HPP
#define STREAM_AUDIO_HPP

#include "audio/wav_writer.hpp"
#include "video/stream/stream.hpp"
#include <vector>

extern "C"
{
	#include <libswresample/swresample.h>
	#include <libavutil/audio_fifo.h>
}

struct StreamAudioSettings
{
	std::string path;
	std::string format;
	AVStream* stream;
	int streamIndex;
	uint32_t sampleRate;
	uint16_t channels;
	int8_t quality;
};

struct WavStreamContext
{
	void createWavAudioWriter();
};

struct OggStreamContext
{
	void loadFormatContext();
	void loadEncoder();
	void createAudioStream();
};

class StreamAudio : public IVidStream
{
private:
	bool isFileClosed{ false };
	int outputLayoutIndex{ 0 };
	
	std::vector<AVChannelLayout> outputLayoutList{ AV_CHANNEL_LAYOUT_MONO, AV_CHANNEL_LAYOUT_STEREO };
	SwrContext* swrContext{ nullptr };
	AVStream* outputStream{ nullptr };

	AVAudioFifo* audioFifo{ nullptr };
	int64_t nextOggPts{ 0 };

	AVFrame* convertedFrame{ nullptr };
	std::unique_ptr<WavWriter> wavWriter;

	StreamAudioSettings settings;

	std::filesystem::path getFilePath(const std::string& format);
	void createAudioCache();
	void saveWavFile();
	void saveOggFile();
	void closeWavFile();
	void closeOggFile();
	void writeEncodedPackets();
	void encodeCompleteFrames();
	void closeFile();
public:
	StreamAudio(StreamAudioSettings settings, ErrorHandler& errorHandler, VideoLifetime& videoLifetime);
	~StreamAudio() override;

	void createWavAudioWriter();
	void createAudioStream();

	void loadDecoder() override;
	void loadEncoder() override;
	void loadFrame() override;
	void loadWavSwrContext();
	void loadOggSwrContext();
	void loadFormatContext();
	void receiveFrame(AVFormatContext*& formatContext) override;
	void saveAsFile() override;
};

#endif
