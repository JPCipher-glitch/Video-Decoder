#include "video/stream/stream_subtitles.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

void StreamSubtitles::loadEncoder() {}

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
    // Close file
    closeFile();
    // Remove the cache directory
    fs::path dirPath = std::format("{}/subtitles", path);
    if (fs::exists(dirPath) && fs::is_directory(dirPath))
        fs::remove_all(dirPath);

    // Cleanup
    av_frame_free(&frame);
    av_packet_free(&sendPacket);
    avcodec_free_context(&decoderContext);
    avsubtitle_free(&subtitle);
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
    {
        invalid = true;
        errorHandler.showError(L"Cannot open audio decoder", false);
        return;
    }
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
    milliseconds = (milliseconds < 0 ? 0 : milliseconds);

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
    if (invalid) return;
    // Allocate the AVPacket & AVFrame
    sendPacket = av_packet_alloc();
    frame = av_frame_alloc();
}

bool StreamSubtitles::writeFrame(AVFormatContext*& formatContext)
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

void StreamSubtitles::receiveFrame(AVFormatContext*& formatContext)
{
    if (invalid) return;

    av_seek_frame(formatContext, -1, 0, AVSEEK_FLAG_BACKWARD); // Reset the reader cursor into the start
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
#pragma region LOAD
void StreamSubtitles::createSubFile()
{
    if (invalid) return;

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
    if (result.starts_with("Dialogue:")) // Remove the prefix
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
std::string StreamSubtitles::extractText()
{
    std::string result;
    for (unsigned int i = 0; i < subtitle.num_rects; i++)
    {
		// Get the subtitle rectangle text
        AVSubtitleRect* rect = subtitle.rects[i];
        if (rect->text) // Check if the text is classic text
        {
			// Add a newline
            if (!result.empty())
                result += "\n";

            // Append the text to the result string
            result += rect->text;
        }
		else if (rect->ass) // Else check if the text is ASS formatted text
        {
            std::string text = extractAssText(rect->ass);
            if (!text.empty())
            {
                // Add a newline
                if (!result.empty())
                    result += "\n";

                // Append the text to the result string
                result += text;
            }
        }
    }

    return result;
}
void StreamSubtitles::saveAsFile()
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
void StreamSubtitles::closeFile()
{
	// Flush the decoder to get the last subs
    AVPacket flushPacket{};
    writeFrame(formatContext);

	// Close the file to complete it
    outputFile.close();
}
#pragma endregion
#pragma endregion
