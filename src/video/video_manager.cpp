#include "video/video_manager.hpp"
#include "video/video_packets.hpp"
#include "video/video_class.hpp"

#include "lifetime.hpp"

#include <iostream>
#include <format>

#pragma region CALLS
void VideoManager::loadVideo(IPacket* packet)
{
	// Load the new video if it don't yet exist
	if (!videoMap.contains(packet->name))
	{
		if (PacketLoad* packetLoad = dynamic_cast<PacketLoad*>(packet))
		{
			videoMap[packetLoad->name] = VideoClass(packetLoad);
		}
	}
	else
	{
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
VideoManager::VideoManager(Lifetime& lifetime) : lifetime(lifetime)
{
	// Fill the call map
	callMap[COMMAND_TYPE::LOAD] = [this](IPacket* packet) { return loadVideo(packet); };
}
#pragma endregion

#pragma region DESTRUCTOR
VideoManager::~VideoManager()
{

}
#pragma endregion

#pragma region UPDATE
void VideoManager::update()
{

}
#pragma endregion