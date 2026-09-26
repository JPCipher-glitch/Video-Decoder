#include "video/video_manager.hpp"

#include <iostream>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <format>
#include <windows.h>

namespace fs = std::filesystem;

#pragma region Creation Methods
#pragma region Private Methods
void VideoManager::removeCache()
{
    // Remove the cache directory
    fs::path dirPath = std::format("{}/.cache", path);
    if (fs::exists(dirPath) && fs::is_directory(dirPath))
        fs::remove_all(dirPath);
}

void VideoManager::openVideo()
{
    // Open video
    avformat_open_input(&formatContext, std::format("{}/{}.{}", path, name, ext).c_str(), nullptr, nullptr);

    // Read stream information
    avformat_find_stream_info(formatContext, nullptr);

    // Find video stream
    for (int i = 0; i < formatContext->nb_streams; i++)
    {
        if (formatContext->streams[i]->codecpar->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex = static_cast<int>(i);
            break;
        }
    }
}

void VideoManager::displayInfo()
{
    // Get video information
    videoStream = formatContext->streams[videoStreamIndex];
    codecParameters = videoStream->codecpar;

    // Find decoder
    codec = avcodec_find_decoder(codecParameters->codec_id);

    // FPS
    double fps = av_q2d(videoStream->avg_frame_rate);

    // Duration
    double duration = static_cast<double>(formatContext->duration) / AV_TIME_BASE;


    // Display information
    std::cout << "Video: " << std::format("{}.{}", name, ext) << '\n';
    std::cout << "Resolution: " << codecParameters->width << "x" << codecParameters->height << '\n';
    std::cout << "Codec: " << codec->long_name << '\n';
    std::cout << "FPS: " << fps << '\n';
    std::cout << "Duration: " << duration << "s" << '\n';
}

void VideoManager::createPacket()
{
    // Create decoder context
    codecContext = avcodec_alloc_context3(codec);
    // Copy codec parameters into codec context
    avcodec_parameters_to_context(codecContext, codecParameters);

    // Create the context to convert the frame into a RGB frame
    swsContext = sws_getContext
    (
        codecContext->width,
        codecContext->height,
        codecContext->pix_fmt,

        codecContext->width,
        codecContext->height,
        AV_PIX_FMT_RGB24,

        SWS_BILINEAR,
        nullptr,
        nullptr,
        nullptr
    );


    // Open decoder
    avcodec_open2(codecContext, codec, nullptr);


    // Allocate packet and frame
    packet = av_packet_alloc();

    // Create the decoded frame
    decodedFrame = av_frame_alloc();

    // Create the RGB frame
    rgbFrame = av_frame_alloc();

    rgbFrame->format = AV_PIX_FMT_RGB24;
    rgbFrame->width = codecContext->width;
    rgbFrame->height = codecContext->height;
    av_frame_get_buffer(rgbFrame, 0);
}
#pragma endregion

// VideoManager Construction
VideoManager::VideoManager(Lifetime* lifetime)
{
    // Get the lifetime engine
    this->lifetime = lifetime;

    // Decode the video
    removeCache();
    openVideo();
    displayInfo();
    createPacket();
}

// VideoManager Destruction
VideoManager::~VideoManager()
{
    // Cleanup
    av_packet_free(&packet);
    av_frame_free(&decodedFrame);
    av_frame_free(&rgbFrame);

    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
}
#pragma endregion

#pragma region Misc Methods
namespace Directory
{
    bool setDirectoryHidden(const std::filesystem::path& path)
    {
        // Get the directory attributes 
        DWORD attributes = GetFileAttributesW(path.c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES) // Check if the attributes are valids
            return false;

        // Enable the "Hidden" attribute to the directory
        attributes |= FILE_ATTRIBUTE_HIDDEN;

        // Return the modified attributes
        return SetFileAttributesW(path.c_str(), attributes);
    }
    void createDirectories(const std::string& path, const std::string& name)
    {
        // Create the .cache directory if it don't yet exist
        fs::path cacheDirectory = std::format("{}/.cache", path);
        if (!fs::exists(cacheDirectory))
            fs::create_directories(cacheDirectory);

        // Hide the cache directory
        setDirectoryHidden(cacheDirectory);

        // Create the video's cache directory if it don't yet exist
        fs::path videoDirectory = std::format("{}/.cache/{}", path, name);
        if (!fs::exists(videoDirectory))
            fs::create_directories(videoDirectory);
    }
}

void saveFrame(const AVFrame* frame, const std::string& path)
{
    // Get the encoder context
    const AVCodec* codec = avcodec_find_encoder(AVCodecID::AV_CODEC_ID_PNG);
    AVCodecContext* encoderContext = avcodec_alloc_context3(codec);

    // Get the settings
    encoderContext->width = frame->width;
    encoderContext->height = frame->height;
    encoderContext->pix_fmt = AV_PIX_FMT_RGB24;
    encoderContext->time_base = { 1, 25 };

    // Create the frame packet
    avcodec_open2(encoderContext, codec, nullptr);
    int result = avcodec_send_frame(encoderContext, frame);

    // Receive the packet
    AVPacket* packet = av_packet_alloc();
    if (result >= 0)
        result = avcodec_receive_packet(encoderContext, packet);
    else
    {
        std::cerr << "Failed to receive PNG packet!" << std::endl;

        av_packet_free(&packet);
        avcodec_free_context(&encoderContext);
        return;
    }

    // Save the frame into an image
    std::ofstream file(path, std::ios::binary);
    file.write(reinterpret_cast<const char*>(packet->data), packet->size);
    file.close();

    // Destroy the packet & the encoder context
    av_packet_free(&packet);
    avcodec_free_context(&encoderContext);
}
#pragma endregion

#pragma region Update Methods
// VideoManager Main Execution
void VideoManager::update()
{
    // Decode frames
    if (av_read_frame(formatContext, packet) >= 0)
    {
        // Ignore audio / other streams
        if (packet->stream_index != videoStreamIndex)
        {
            av_packet_unref(packet);
            return;
        }

        // Send compressed packet to decoder
        avcodec_send_packet(codecContext, packet);

        // Receive decoded frames
        while (true)
        {
            int result = avcodec_receive_frame(codecContext, decodedFrame);

            if (result == AVERROR(EAGAIN) || result == AVERROR_EOF)
                break;

            // Convert the YUV frame into a RBG frame
            sws_scale
            (
                swsContext,

                decodedFrame->data,
                decodedFrame->linesize,

                0,
                decodedFrame->height,

                rgbFrame->data,
                rgbFrame->linesize
            );

            // Create the directories
            Directory::createDirectories(path, name);
            // Save the frame into a PNG image
            std::string frameName = std::format("{}/.cache/{}/frame_{:06}.png", path, name, frameCount);
            saveFrame(rgbFrame, frameName);
            frameCount++;
        }
        av_packet_unref(packet);

        // Just for testing
        /*if (frameCount >= 10)
            break;*/
    }
    else
    {
        lifetime->destroy();
    }
}
#pragma endregion