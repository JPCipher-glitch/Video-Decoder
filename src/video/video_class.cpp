#include "video/video_class.hpp"
#include "lifetime.hpp"

#include <iostream>
#include <filesystem>
#include <format>

namespace fs = std::filesystem;

#pragma region STREAMS
void VideoClass::loadContext()
{
	// Open video
	std::lock_guard<std::mutex> lock(mutex);
	if (avformat_open_input(&stuff.formatContext, std::format("{}{}", settings.path, settings.videoFormat).c_str(), nullptr, nullptr) < 0)
	{
		std::cerr << std::format("ERROR: Invalid video path: {}{}\n", settings.path, settings.videoFormat);
	}
}

void VideoClass::loadStreams()
{
	if (!stuff.formatContext) return;

	// Read stream information
	{
		std::lock_guard<std::mutex> lock(mutex);
		avformat_find_stream_info(stuff.formatContext, nullptr);
	}

	// Find video stream
	for (int i = 0; i < stuff.formatContext->nb_streams; i++)
	{
		AVMediaType type = stuff.formatContext->streams[i]->codecpar->codec_type;
		if (stuff.availableStreamType.contains(type) && !stuff.videoStreamMap.contains(type)) // Add each available stream necessary to extract data into the video file
		{
			std::lock_guard<std::mutex> lock(mutex);
			
			AVStream* stream = stuff.formatContext->streams[i];
			stuff.videoStreamMap[type] = stream;
		}
	}
}
#pragma endregion

#pragma region INFOS
void VideoClass::displayInfo()
{
	if (!stuff.formatContext) return;

	// Get video information
	AVStream* videoStream = stuff.videoStreamMap[AVMediaType::AVMEDIA_TYPE_VIDEO];
	AVCodecParameters* codecParameters = videoStream->codecpar;

	// Find decoder
	const AVCodec* codec = avcodec_find_decoder(codecParameters->codec_id);

	// FPS
	double fps = av_q2d(videoStream->avg_frame_rate);

	// Duration
	double duration = static_cast<double>(stuff.formatContext->duration) / AV_TIME_BASE;


	// Display information
	std::cout << "Video: " << std::format("{}{}", settings.path, settings.videoFormat) << '\n';
	std::cout << "Resolution: " << codecParameters->width << "x" << codecParameters->height << '\n';
	std::cout << "Codec: " << codec->long_name << '\n';
	std::cout << "FPS: " << fps << '\n';
	std::cout << "Duration: " << duration << "s" << '\n';
}
#pragma endregion


#pragma region CACHE
void VideoClass::createCacheFolder()
{
	// Create the video's cache directory if it don't yet exist
	fs::path videoDirectory = std::format("{}/.cache/{}", path, settings.name);
	if (!fs::exists(videoDirectory))
		fs::create_directories(videoDirectory);
}
#pragma endregion

#pragma region CONSTRUCTOR
VideoClass::VideoClass(const std::string& path, PacketLoad* packet) : path{ path }, settings { *packet }
{
	createCacheFolder();

	loadContext();
	loadStreams();
	displayInfo();

	std::cout << std::format("VIDEO LOADED: {} -> {}{}\n", settings.name, settings.path, settings.videoFormat);
}
#pragma endregion

#pragma region DESTRUCTOR
VideoClass::~VideoClass()
{
	// Cleanup
	av_packet_free(&stuff.packet);
	av_frame_free(&stuff.decodedFrame);
	av_frame_free(&stuff.rgbFrame);

	avcodec_free_context(&stuff.codecContext);
	avformat_close_input(&stuff.formatContext);
}
#pragma endregion

#pragma region UPDATE

#pragma endregion