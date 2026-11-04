#include "video/stream/stream_subtitles.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

#pragma region CONSTRUCTOR
StreamSubtitles::StreamSubtitles(StreamSubSettings s, ErrorHandler& errorHandler) : settings{ s }, IVidStream{ errorHandler, s.path, s.stream, s.streamIndex }
{
    createSubsCache();
    loadDecoder();
    loadFrame();
    createSubFile();
}
#pragma endregion

#pragma region DESTRUCTOR
StreamSubtitles::~StreamSubtitles()
{
    // Remove the cache directory
    fs::path dirPath = std::format("{}/subtitles", path);
    if (fs::exists(dirPath) && fs::is_directory(dirPath))
        fs::remove_all(dirPath);

    // Cleanup
    av_frame_free(&frame);
    av_packet_free(&sendPacket);
    av_packet_free(&receivePacket);
    avcodec_free_context(&decoderContext);
}
#pragma endregion

#pragma region DECODER
void StreamSubtitles::loadDecoder()
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

#pragma region CACHE
void StreamSubtitles::createSubsCache()
{
    // Create the video's cache directory if it don't yet exist
    fs::path audioDirectory = std::format("{}/subtitles", path);
    if (!fs::exists(audioDirectory))
        fs::create_directories(audioDirectory);
}
#pragma endregion


#pragma region FILE PATH
fs::path StreamSubtitles::getFilePath(const std::string& format)
{
    AVDictionaryEntry* language = av_dict_get(stream->metadata, "language", nullptr, 0);
    fs::path filePath = std::format("{}/subtitles/subtitle_{}.{}", path, language->value, format);
    return filePath;
}
#pragma endregion

#pragma region TIME FORMAT
std::string StreamSubtitles::formatTimestamp(int64_t milliseconds)
{
    if (milliseconds < 0)
        milliseconds = (milliseconds >= 0 ? milliseconds : 0);

    int64_t hours = milliseconds / 3600000;
    int64_t minutes = (milliseconds / 60000) % 60;
    int64_t seconds = (milliseconds / 1000) % 60;
    int64_t ms = milliseconds % 1000;

    std::ostringstream timestamp;
    timestamp << std::setfill('0') << std::setw(2) << hours << ":" << std::setw(2) << minutes << ":" << std::setw(2) << seconds << "," << std::setw(3) << ms;

    return timestamp.str();
}
#pragma endregion


#pragma region FRAME
void StreamSubtitles::loadFrame()
{
    // Allocate the AVPacket & AVFrame
    sendPacket = av_packet_alloc();
    receivePacket = av_packet_alloc();
    frame = av_frame_alloc();
}

void StreamSubtitles::receiveFrame(AVFormatContext*& formatContext)
{
    av_seek_frame(formatContext, -1, 0, AVSEEK_FLAG_BACKWARD); // Reset the reader cursor into the start
    avcodec_flush_buffers(decoderContext); // Wipe the last buffers before reading another sample

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

    // Close the audio file to complete it
    closeFile();
}
#pragma endregion

#pragma region FILE
#pragma region LOAD
void StreamSubtitles::createSubFile()
{
    if (!outputFile.is_open())
    {
        fs::path filePath = getFilePath("srt");
        outputFile.open(filePath, std::ios::binary);
    }
}
#pragma endregion
#pragma region BASE FILE
std::string StreamSubtitles::extractAssText(const char* ass)
{
    std::string result = ass;
    if (result.starts_with("Dialogue:"))
    {
        result.erase(0, result.find(':') + 1);
        if (!result.empty() && result[0] == ' ')
            result.erase(0, 1);
    }

    for (int i = 0; i < 8; i++)
    {
        size_t separator = result.find(',');
        if (separator == std::string::npos)
            return "";

        result.erase(0, separator + 1);
    }

    std::string cleaned;
    bool insideTag = false;

    for (size_t i = 0; i < result.size(); i++)
    {
        char c = result[i];

        if (c == '{')
        {
            insideTag = true;
            continue;
        }

        if (c == '}')
        {
            insideTag = false;
            continue;
        }

        if (insideTag)
            continue;

        if (c == '\\' && i + 1 < result.size())
        {
            char next = result[i + 1];
            if (next == 'N' || next == 'n')
            {
                cleaned += '\n';
                i++;
                continue;
            }

            if (next == 'h')
            {
                cleaned += ' ';
                i++;
                continue;
            }
        }
        cleaned += c;
    }

    return cleaned;
}
std::string StreamSubtitles::extractText(const AVSubtitle& subtitle)
{
    std::string result;
    for (unsigned int i = 0; i < subtitle.num_rects; i++)
    {
        AVSubtitleRect* rect = subtitle.rects[i];
        if (rect->text)
        {
            if (!result.empty())
                result += "\n";

            result += rect->text;
        }
        else if (rect->ass)
        {
            std::string text = extractAssText(rect->ass);
            if (!text.empty())
            {
                if (!result.empty())
                    result += "\n";

                result += text;
            }
        }
    }

    return result;
}
void StreamSubtitles::saveAsFile()
{
    std::string text = extractText(subtitle);

    if (text.empty())
    {
        return;
    }

    int64_t basePts = subtitle.pts;

    if (basePts == AV_NOPTS_VALUE &&
        packet &&
        packet->pts != AV_NOPTS_VALUE)
    {
        basePts = av_rescale_q(
            packet->pts,
            stream->time_base,
            AV_TIME_BASE_Q
        );
    }

    if (basePts == AV_NOPTS_VALUE)
    {
        return;
    }

    int64_t startTime =
        basePts / 1000 + subtitle.start_display_time;

    int64_t endTime =
        basePts / 1000 + subtitle.end_display_time;

    if (endTime <= startTime)
    {
        if (!packet || packet->duration <= 0)
        {
            return;
        }

        endTime = startTime + av_rescale_q(
            packet->duration,
            stream->time_base,
            AVRational{ 1, 1000 }
        );
    }

    outputFile << subtitleIndex++ << "\r\n" << formatTimestamp(startTime) << " --> " << formatTimestamp(endTime) << "\r\n" << text << "\r\n\r\n";
}
#pragma endregion
#pragma region CLOSE FILE
void StreamSubtitles::closeFile()
{
    if (settings.format == ".wav")
        closeWavFile();
    else if (settings.format == ".ogg")
        closeOggFile();
}
#pragma endregion
#pragma endregion
