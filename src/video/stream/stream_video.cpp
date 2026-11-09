#include "video/stream/stream_video.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

#pragma region CONSTRUCTOR
StreamVideo::StreamVideo(StreamFrameSettings s, ErrorHandler& errorHandler) : settings{ s }, IVidStream{ errorHandler, s.path, s.stream, s.streamIndex }
{
    createFramesCache();
    loadDecoder();
    loadEncoder();
    loadFrame();
    loadSwsContext();
    createExportFrame();
}
#pragma endregion

#pragma region DESTRUCTOR
StreamVideo::~StreamVideo()
{
    // Remove the cache directory
    fs::path dirPath = std::format("{}/frames", path);
    if (fs::exists(dirPath) && fs::is_directory(dirPath))
        fs::remove_all(dirPath);

    // Cleanup
    av_frame_free(&frame);
    av_packet_free(&sendPacket);
    av_packet_free(&receivePacket);
    avcodec_free_context(&decoderContext);
    avcodec_free_context(&encoderContext);
}
#pragma endregion

#pragma region DECODER
void StreamVideo::loadDecoder()
{
    // Find a registered decoder
    decoder = avcodec_find_decoder(stream->codecpar->codec_id);

    // Create decoder context
    decoderContext = avcodec_alloc_context3(decoder);
    avcodec_parameters_to_context(decoderContext, stream->codecpar); // Copy codec parameters into codec context

    // Open decoder
    if (avcodec_open2(decoderContext, decoder, nullptr) < 0)
    {
        invalid = true;
        errorHandler.showError(L"Cannot open video decoder", false);
        return;
    }
}
#pragma endregion

#pragma region ENCODER
void StreamVideo::loadEncoder()
{
    // Find the encoder context
    encoder = avcodec_find_encoder(AVCodecID::AV_CODEC_ID_PNG);

    // Create the PNG encoder
    encoderContext = avcodec_alloc_context3(encoder);

    // Get the settings
    encoderContext->width = settings.width;
    encoderContext->height = settings.height;
    encoderContext->pix_fmt = AV_PIX_FMT_RGB24;
    encoderContext->time_base = { 1, 25 };

    // Open encoder
    avcodec_open2(encoderContext, encoder, nullptr);
}
#pragma endregion

#pragma region SWS CONTEXT
void StreamVideo::loadSwsContext()
{
    // Create the context to convert the frame into a RGB frame
    swsContext = sws_getContext
    (
        decoderContext->width,
        decoderContext->height,
        decoderContext->pix_fmt,

        settings.width,
        settings.height,
        AV_PIX_FMT_RGB24,

        SWS_BILINEAR,
        nullptr,
        nullptr,
        nullptr
    );
}
#pragma endregion

#pragma region EXPORT FRAME
void StreamVideo::createExportFrame()
{
    // Create the RGB frame
    exportFrame = av_frame_alloc();

    exportFrame->format = AV_PIX_FMT_RGB24;
    exportFrame->width = encoderContext->width;
    exportFrame->height = encoderContext->height;
    av_frame_get_buffer(exportFrame, 0);
}
#pragma endregion


#pragma region CACHE
void StreamVideo::createFramesCache()
{
    // Create the video's cache directory if it don't yet exist
    fs::path audioDirectory = std::format("{}/frames", path);
    if (!fs::exists(audioDirectory))
        fs::create_directories(audioDirectory);
}
#pragma endregion


#pragma region FILE PATH
fs::path StreamVideo::getFilePath(const std::string& format)
{
    fs::path filePath = std::format("{}/frames/frame_{:06}{}", path, frameIndex, format);
    return filePath;
}
#pragma endregion


#pragma region FRAME
void StreamVideo::loadFrame()
{
    if (invalid) return;
    // Allocate the AVPacket & AVFrame
    sendPacket = av_packet_alloc();
    frame = av_frame_alloc();
    receivePacket = av_packet_alloc();
}

bool StreamVideo::writeFrame(AVFormatContext*& formatContext)
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
                return true;

            // Create the audio file
            saveAsFile();

            // Wipe the packet to reusing it
            av_frame_unref(frame);
        }
    }
    av_packet_unref(sendPacket); // Wipe the packet to reusing it
    return true;
}

void StreamVideo::receiveFrame(AVFormatContext*& formatContext)
{
    if (invalid) return;

    av_seek_frame(formatContext, streamIndex, 0, AVSEEK_FLAG_BACKWARD); // Reset the reader cursor into the start
    avcodec_flush_buffers(decoderContext); // Wipe the last buffers before reading another sample

    // Decode frames
    while (av_read_frame(formatContext, sendPacket) >= 0)
    {
        if (!writeFrame(formatContext))
            break;
    }

    // Close the subs file to complete it
    closeFile();
}
#pragma endregion

#pragma region FILE
void StreamVideo::saveAsFile()
{
    // Rescale the export frame with the sws ctx
    sws_scale
    (
        swsContext,

        frame->data,
        frame->linesize,

        0,
        frame->height,

        exportFrame->data,
        exportFrame->linesize
    );

    // Create the frame packet
    int result = avcodec_send_frame(encoderContext, frame);
    if (result >= 0)
        result = avcodec_receive_packet(encoderContext, receivePacket);
    else
    {
        std::cerr << "Failed to receive PNG packet!" << std::endl;
        return;
    }

    // Save the frame into an image
    std::ofstream file(getFilePath(settings.format), std::ios::binary);
    file.write(reinterpret_cast<const char*>(receivePacket->data), receivePacket->size);
    file.close();

    // Wipe the packet to reusing it
    av_packet_unref(receivePacket);

    frameIndex++;
}
#pragma endregion
#pragma region CLOSE FILE
void StreamVideo::closeFile()
{

}
#pragma endregion
#pragma endregion
