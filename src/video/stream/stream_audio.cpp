#include "video/stream/stream_audio.hpp"

#include <iostream>
#include <filesystem>

#pragma region CONSTRUCTOR
StreamAudio::StreamAudio(std::string path, AVStream* stream, int streamIndex) : IStream{ path, stream, streamIndex } 
{
    std::filesystem::path filePath = std::format("{}.wav", path);
    filePath.replace_filename("audio.wav");
    wavWriter = std::make_unique<WavWriter>(filePath.string(), 44100, 2);

    loadDecoder();
    loadEncoder();
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
    //// Find a registered encoder
    //encoder = avcodec_find_encoder(AVCodecID::AV_CODEC_ID_PCM_S16LE);

    //// Create encoder context
    //encoderContext = avcodec_alloc_context3(encoder);

    //// Open encoder
    //if (avcodec_open2(encoderContext, encoder, nullptr) < 0)
    //    throw std::runtime_error("Cannot open audio encoder");
}
#pragma endregion

#pragma region SWR CONTEXT
void StreamAudio::loadSwrContext()
{
    swr_alloc_set_opts2
    (
        &swrContext,
        &outputLayout,
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
    av_channel_layout_copy(&convertedFrame->ch_layout, &outputLayout);
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


    // S16 interleaved, stereo: 2 channels * 2 bytes per sample
    uint32_t pcmSize = static_cast<uint32_t>(samples * 2 * sizeof(int16_t));
    wavWriter->writeSamples(convertedFrame->data[0], pcmSize);
}
#pragma endregion