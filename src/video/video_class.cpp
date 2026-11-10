#include "video/video_class.hpp"
#include "lifetime.hpp"

#include "video/stream/stream_audio.hpp"
#include "video/stream/stream_subtitles.hpp"
#include "video/stream/stream_video.hpp"

#include <iostream>
#include <filesystem>
#include <format>

namespace fs = std::filesystem;

#pragma region CONSTRUCTOR
VideoClass::VideoClass(ErrorHandler& errorHandler) : errorHandler{ errorHandler } {}
#pragma endregion


#pragma region ASSETS
void VideoClass::loadAssets()
{
	// Don't load the assets if the video is already loading
	if (isLoading.exchange(true))
		return;

	isLoading = true; // Set the loading state to true
	streamTimeStart = std::chrono::duration<float>(std::chrono::steady_clock::now().time_since_epoch()).count();

	loadingTask = std::jthread([this](std::stop_token stopToken)
	{
		for (auto& video : stuff.videoStreamMap)
		{
			// Stop the loading if requested
			if (stopToken.stop_requested())
				return;

			video.second->receiveFrame(stuff.formatContext); // Receive all the frames from the video stream
		}

		isLoaded = true;
		isLoading = false;
	});
}
#pragma endregion

#pragma region STREAMS
void VideoClass::loadContext()
{
	if (!valid) return;
	// Open video
	if (avformat_open_input(&stuff.formatContext, std::format("{}{}", settings.path, settings.videoFormat).c_str(), nullptr, nullptr) < 0)
	{
		valid = false;
		errorHandler.showError(std::format(L"The following video was not found in this path:\n\"{}{}\"", errorHandler.toWString(settings.path), errorHandler.toWString(settings.videoFormat)), false);
		std::cerr << std::format("ERROR: Invalid video path: {}{}\n", settings.path, settings.videoFormat);
	}
}

void VideoClass::loadStreams()
{
	if (!valid) return;
	if (!stuff.formatContext) return;

	// Read stream information
	avformat_find_stream_info(stuff.formatContext, nullptr);

	// Find video stream
	for (int i = 0; i < stuff.formatContext->nb_streams; i++)
	{
		AVMediaType type = stuff.formatContext->streams[i]->codecpar->codec_type;
		if (stuff.availableStreamType.contains(type) && loadStreamMap.contains(type)) // Add each available stream necessary to extract data into the video file
		{	
			AVStream* stream = stuff.formatContext->streams[i];
			stuff.videoStreamMap.insert(std::make_pair(type, loadStreamMap[type](stream, i)));
		}
	}
}
#pragma endregion

#pragma region VALID
bool VideoClass::isValid()
{
	return valid;
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
	if (!valid) return;

	// Create the video's cache directory if it don't yet exist
	fs::path videoDirectory = std::format("{}/.cache/{}", path, settings.name);
	if (!fs::exists(videoDirectory))
	{
		try
		{
			fs::create_directories(videoDirectory);
		}
		catch (const std::filesystem::filesystem_error& e)
		{
			valid = false;
			errorHandler.showError(std::format(L"Failed to create the cache directory \"{}\"", errorHandler.toWString(videoDirectory.string())), false);
		}
	}
}
#pragma endregion

#pragma region CONSTRUCTOR
VideoClass::VideoClass(const std::string& path, PacketLoad* packet, ErrorHandler& handler) : path{ path }, settings{ *packet }, errorHandler{ handler }
{
	// Fill the load map
	loadStreamMap.emplace(AVMediaType::AVMEDIA_TYPE_VIDEO, [this](AVStream* stream, int streamIndex)
	{
		StreamFrameSettings frameSettings{ std::format("{}/.cache/{}", this->path, settings.name), settings.frameFormat, stream, streamIndex, settings.width, settings.height, settings.fps };
		auto s = std::make_unique<StreamVideo>(frameSettings, errorHandler);
		return std::move(s);
	});
	loadStreamMap.emplace(AVMediaType::AVMEDIA_TYPE_AUDIO, [this](AVStream* stream, int streamIndex) 
	{
		StreamAudioSettings audioSettings{ std::format("{}/.cache/{}", this->path, settings.name), settings.audioFormat, stream, streamIndex, settings.audioSampleRate, settings.audioChannels, settings.audioQuality };
		auto s = std::make_unique<StreamAudio>(audioSettings, errorHandler);
		return std::move(s);
	});
	loadStreamMap.emplace(AVMediaType::AVMEDIA_TYPE_SUBTITLE, [this](AVStream* stream, int streamIndex)
	{
		StreamSubSettings subSettings{ std::format("{}/.cache/{}", this->path, settings.name), stream, streamIndex };
		auto s = std::make_unique<StreamSubtitles>(subSettings, errorHandler);
		return std::move(s);
	});

	if (valid)
	{
		double now = std::chrono::duration<float>(std::chrono::steady_clock::now().time_since_epoch()).count();
		double duration = now - settings.timePoint;
		std::cout << std::format("VIDEO LOADED: {} -> {}{} in {}s\n", settings.name, settings.path, settings.videoFormat, duration);
	}
}
#pragma endregion

#pragma region DESTRUCTOR
VideoClass::~VideoClass()
{
	loadingTask.request_stop();

	// Remove the cache directory
	fs::path dirPath = std::format("{}/.cache/{}", path, settings.name);
	if (fs::exists(dirPath) && fs::is_directory(dirPath))
		fs::remove_all(dirPath);
	avformat_close_input(&stuff.formatContext);
}
#pragma endregion

#pragma region LOAD
void VideoClass::load()
{
	if (!valid)
		return;

	createCacheFolder();
	loadContext();
	loadStreams();
	loadAssets();
}
#pragma endregion


#pragma region UPDATE
void VideoClass::update()
{
	if (!initialized)
		load();

	initialized = true;

	if (!isLoaded) return;

	// LOG
	if (!islog)
	{
		double now = std::chrono::duration<float>(std::chrono::steady_clock::now().time_since_epoch()).count();
		double duration = now - streamTimeStart;

		std::cout << std::format("STREAMS LOADED in {}s\n", duration);
		islog = true;
	}

	// Erase all the invalid streams that got an error
	for (auto video = stuff.videoStreamMap.begin(); video != stuff.videoStreamMap.end();)
	{
		if (video->second->isInvalid())
			video = stuff.videoStreamMap.erase(video);
		else
			video++;
	}
}
#pragma endregion