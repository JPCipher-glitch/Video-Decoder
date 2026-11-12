#include "video/video_manager.hpp"
#include "video/video_class.hpp"
#include "video/video_packets.hpp"

#include "lifetime.hpp"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

#pragma region CALLS
void VideoManager::loadVideo(IPacket* packet)
{
	// Load the new video if it don't yet exist
	if (!videoMap.contains(packet->name))
	{
		if (PacketLoad* packetLoad = dynamic_cast<PacketLoad*>(packet))
			videoMap.emplace(packetLoad->name, std::make_unique<VideoClass>(path, packetLoad, errorHandler));
	}
	else
	{
		errorHandler.showWarning(std::format(L"This video was already loaded\nor have the same name with another one:\n\"{}\"", errorHandler.toWString(packet->name)), false);
		std::cout << std::format("ERROR: VIDEO ALREADY LOADED: {}\n", packet->name);
	}
}

void VideoManager::call(IPacket* packet)
{
	// Call the command asked & check if the command exist
	if (callMap.contains(packet->type))
		callMap[packet->type](packet);
}
#pragma endregion

#pragma region CONSTRUCTOR
void VideoManager::setPath(const std::string& p)
{
	fs::path videoDirectory = p;
	if (fs::exists(videoDirectory))
		path = p;
}

VideoManager::VideoManager(const std::string& path, Lifetime& lifetime, ErrorHandler& errorHandler) : path(path), lifetime(lifetime), errorHandler(errorHandler)
{
	// Fill the call map
	callMap[COMMAND_TYPE::LOAD] = [this](IPacket* packet) { return loadVideo(packet); };
}
#pragma endregion

#pragma region DESTRUCTOR
VideoManager::~VideoManager()
{
	// Request stop for every VideoClass
	for (auto& video : videoMap)
		video.second->requestStop();

	// Wait for every loading thread
	for (auto& video : videoMap)
		video.second->join();

	// Clear the video map
	videoMap.clear();

	// Remove the cache directory
	fs::path dirPath = std::format("{}/.cache", path);
	if (fs::exists(dirPath) && fs::is_directory(dirPath))
		fs::remove_all(dirPath);
}
#pragma endregion

#pragma region UPDATE
void VideoManager::update()
{
	for (auto video = videoMap.begin(); video != videoMap.end();)
	{
		if (video->second->isValid())
		{
			video->second->update();
			video++;
		}
		else
		{
			video = videoMap.erase(video);
		}
	}
}
#pragma endregion