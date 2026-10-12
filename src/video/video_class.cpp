#include "video/video_class.hpp"
#include "lifetime.hpp"

#include "video/stream/stream_audio.hpp"

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
		if (stuff.availableStreamType.contains(type) && !stuff.videoStreamMap.contains(type) && loadStreamMap.contains(type)) // Add each available stream necessary to extract data into the video file
		{
			std::lock_guard<std::mutex> lock(mutex);
			
			AVStream* stream = stuff.formatContext->streams[i];
			stuff.videoStreamMap[type] = loadStreamMap[type](stream, i);
		}
	}
}
#pragma endregion

#pragma region INFOS
void VideoClass::displayInfo()
{
	
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
	// Fill the load map
	{
		std::lock_guard<std::mutex> lock(mutex);
		loadStreamMap.emplace(AVMediaType::AVMEDIA_TYPE_AUDIO, [this](AVStream* stream, int streamIndex) {return std::make_unique<StreamAudio>(std::format("{}/.cache/{}", this->path, settings.name), stream, streamIndex); });
	}

	createCacheFolder();

	loadContext();
	loadStreams();

	std::cout << std::format("VIDEO LOADED: {} -> {}{}\n", settings.name, settings.path, settings.videoFormat);
}
#pragma endregion

#pragma region DESTRUCTOR
VideoClass::~VideoClass()
{
	avformat_close_input(&stuff.formatContext);
}
#pragma endregion

#pragma region LOAD
void VideoClass::load()
{
	if (loadStreamMap.contains(AVMediaType::AVMEDIA_TYPE_AUDIO))
	{
		StreamAudio* stream = dynamic_cast<StreamAudio*>(stuff.videoStreamMap[AVMediaType::AVMEDIA_TYPE_AUDIO].get());
		stream->receiveFrame(stuff.formatContext);
	}
}
#pragma endregion


#pragma region UPDATE
void VideoClass::update()
{
	if (!initialized)
		load();

	initialized = true;
}
#pragma endregion