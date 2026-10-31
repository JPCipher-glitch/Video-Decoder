#include "video/stream/stream_audio.hpp"

#include <iostream>
#include <filesystem>

#pragma region CONSTRUCTOR
StreamAudio::StreamAudio(std::string path, AVStream* stream, int streamIndex) : IVidStream{ path, stream, streamIndex } 
{
    loadDecoder();
    //loadEncoder();
    loadSwrContext();
    loadFrame();
};
#pragma endregion

#pragma region DESTRUCTOR
StreamAudio::~StreamAudio()
{
    // Cleanup
    av_frame_free(&frame);
    av_frame_free(&convertedFrame);
    av_packet_free(&sendPacket);
    av_packet_free(&receivePacket);
    avcodec_free_context(&decoderContext);
    avcodec_free_context(&encoderContext);
};
#pragma endregion

#pragma region WRITER
void StreamAudio::createAudioWriter(uint32_t sampleRate, uint16_t channels)
{
    audioSampleRate = sampleRate;
    audioChannels = channels;

    std::filesystem::path filePath = std::format("{}.wav", path);
    filePath.replace_filename("audio.wav");
    wavWriter = std::make_unique<WavWriter>(filePath.string(), audioSampleRate, audioChannels);

    if (channels <= outputLayoutList.size())
        outputLayoutIndex = static_cast<int>(channels) - 1;
    else
        throw std::runtime_error("ERROR: Invalid channels");
}
#pragma endregion


#pragma region DECODER
void StreamAudio::loadDecoder()
{
    // Find a registered decoder
    decoder = avcodec_find_decoder(stream->codecpar->codec_id);

    // Create decoder context
    decoderContext = avcodec_alloc_context3(decoder);
    avcodec_parameters_to_context(decoderContext, stream->codecpar); // Copy codec parameters into codec context

    // Open decoder
    if (avcodec_open2(decoderContext, decoder, nullptr) < 0)
        throw std::runtime_error("Cannot open audio decoder");
}
#pragma endregion

#pragma region DECODER
void StreamAudio::loadEncoder()
{
    uint32_t sampleRate{ 0 }; uint16_t channels{0};
    
    // Find a registered encoder
    encoder = avcodec_find_encoder(AV_CODEC_ID_VORBIS);

    if (encoder == nullptr)
        throw std::runtime_error("Cannot find Vorbis encoder");

    // Create encoder context
    encoderContext = avcodec_alloc_context3(encoder);

    // Configure encoder
    encoderContext->sample_rate = static_cast<int>(sampleRate);
    encoderContext->sample_fmt = AV_SAMPLE_FMT_FLTP;
    encoderContext->time_base = AVRational{ 1, encoderContext->sample_rate };

    av_channel_layout_default(&encoderContext->ch_layout, channels);

    // Compression quality: 0 to 10
    encoderContext->flags |= AV_CODEC_FLAG_QSCALE;
    encoderContext->global_quality = 5 * FF_QP2LAMBDA;

    // Open encoder
    if (avcodec_open2(encoderContext, encoder, nullptr) < 0)
        throw std::runtime_error("Cannot open audio encoder");
}
#pragma endregion

#pragma region SWR CONTEXT
void StreamAudio::loadSwrContext()
{
    swr_alloc_set_opts2
    (
        &swrContext,
        &outputLayoutList[outputLayoutIndex],
        AV_SAMPLE_FMT_S16,
        44100,
        &decoderContext->ch_layout,
        decoderContext->sample_fmt,
        decoderContext->sample_rate,
        0,
        nullptr
    );

    swr_init(swrContext);
}
#pragma endregion

#pragma region AUDIO FORMAT
void StreamAudio::loadFormatContext()
{
    // Load the format context to Write the OGG audio file
    std::filesystem::path filePath = std::format("{}.ogg", path);
    filePath.replace_filename("audio.ogg");
    int result = avformat_alloc_output_context2(&formatContext, nullptr, ".ogg", filePath.string().c_str());

    // Check if the ogg format context was normaly allocated
    if (result < 0)
        throw std::runtime_error("ERROR: Cannot allocate OGG format context");
}
#pragma endregion



#pragma region FRAME
void StreamAudio::loadFrame()
{
    // Allocate the AVPacket & AVFrame
    sendPacket = av_packet_alloc();
    receivePacket = av_packet_alloc();
    frame = av_frame_alloc();
    convertedFrame = av_frame_alloc();
}

void StreamAudio::receiveFrame(AVFormatContext*& formatContext)
{
    // Decode frames
    while (av_read_frame(formatContext, sendPacket) >= 0)
    {
        // Get the audio stream
        if (sendPacket->stream_index == streamIndex)
        {
            // Send compressed packet to decoder
            avcodec_send_packet(decoderContext, sendPacket);

            // Receive decoded frames
            while (true)
            {
                // Receive the decoded data frame
                int result = avcodec_receive_frame(decoderContext, frame);
                if (result == AVERROR(EAGAIN) || result == AVERROR_EOF)
                    break;

                // Create the audio file
                saveAsFile();

                // Wipe the packet to reusing it
                av_frame_unref(frame);
            }
        }
        av_packet_unref(sendPacket); // Wipe the packet to reusing it
    }

    wavWriter->close();
}
#pragma endregion

#pragma region FILE
void StreamAudio::saveAsFile()
{
    int64_t nbSamples = av_rescale_rnd
    (
        swr_get_delay(swrContext, decoderContext->sample_rate) + frame->nb_samples,
        44100,
        decoderContext->sample_rate,
        AVRounding::AV_ROUND_UP
    );
    av_frame_unref(convertedFrame);

    convertedFrame->format = AV_SAMPLE_FMT_S16;
    convertedFrame->sample_rate = 44100;
    av_channel_layout_copy(&convertedFrame->ch_layout, &outputLayoutList[outputLayoutIndex]);
    convertedFrame->nb_samples = static_cast<int>(nbSamples);

    // Prepare the PCM buffer
    if (av_frame_get_buffer(convertedFrame, 0) < 0)
        throw std::runtime_error("ERROR: Cannot allocate PCM buffer");

    // Convert the PCM buffer into Audio samples
    int samples = swr_convert
    (
        swrContext,
        convertedFrame->data,
        convertedFrame->nb_samples,
        const_cast<const uint8_t**>(frame->extended_data),
        frame->nb_samples
    );
    if (samples < 0)
        throw std::runtime_error("Cannot convert audio samples");

    if (samples == 0)
        return;

    // PCM size
    uint32_t pcmSize = static_cast<uint32_t>(samples * audioChannels * sizeof(int16_t));
    wavWriter->writeSamples(convertedFrame->data[0], pcmSize);
}
#pragma endregion