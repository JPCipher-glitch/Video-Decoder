#include "video/stream/stream_video.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

void StreamVideo::loadEncoder() {}

#pragma region CONSTRUCTOR
StreamVideo::StreamVideo(StreamFrameSettings s, ErrorHandler& errorHandler) : settings{ s }, IVidStream{ errorHandler, s.path, s.stream, s.streamIndex }
{
    createFramesCache();
    loadDecoder();
    loadFrame();
    loadSwsContext();
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
    avcodec_free_context(&decoderContext);
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
    AVDictionaryEntry* language = av_dict_get(stream->metadata, "language", nullptr, 0);
    fs::path filePath = std::format("{}/frames/frame_{}.{}", path, language->value, format);
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
}

bool StreamVideo::writeFrame(AVFormatContext*& formatContext)
{
    // Get the subs stream
    if (sendPacket->stream_index == streamIndex)
    {
        // Decode the subtitle packet
        int result = avcodec_decode_subtitle2(decoderContext, &subtitle, &gotSubtitle, sendPacket);
        if (result == AVERROR(EAGAIN) || result == AVERROR_EOF)
            return false;

        // Save the subtitle to file if it was successfully decoded
        if (gotSubtitle)
            saveAsFile();
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
	// Extract the subtitle text from the AVSubtitle struct
    std::string text = extractText();
    if (text.empty())
        return;

	// Calculate the base PTS
    int64_t basePts = subtitle.pts;
    if (basePts == AV_NOPTS_VALUE && sendPacket->pts != AV_NOPTS_VALUE)
        basePts = av_rescale_q(sendPacket->pts, stream->time_base, AV_TIME_BASE_Q);

    if (basePts == AV_NOPTS_VALUE)
        return;

	// Get the start and end timestamps
    int64_t startTime = basePts / 1000 + subtitle.start_display_time;
    int64_t endTime = basePts / 1000 + subtitle.end_display_time;

	// If the end time is less than or equal to the start time, use the packet duration to calculate the end time
    if (endTime <= startTime)
    {
        if (!sendPacket || sendPacket->duration <= 0)
            return;

        endTime = startTime + av_rescale_q(sendPacket->duration, stream->time_base, AVRational{ 1, 1000 });
    }
    subsIndex++;

    // Write the sub line into the file
    outputFile << subsIndex << "\r\n" << formatTimestamp(startTime) << " --> " << formatTimestamp(endTime) << "\r\n" << text << "\r\n\r\n";
}
#pragma endregion
#pragma region CLOSE FILE
void StreamVideo::closeFile()
{
	// Flush the decoder to get the last subs
    AVPacket flushPacket{};
    writeFrame(formatContext);

	// Close the file to complete it
    outputFile.close();
}
#pragma endregion
#pragma endregion
