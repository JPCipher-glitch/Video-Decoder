#include "video/video_class.hpp"
#include "lifetime.hpp"

#include <iostream>

#pragma region CONSTRUCTOR
VideoClass::VideoClass(PacketLoad* packet) : settings{ *packet }
{
	std::cout << std::format("VIDEO LOADED: {} -> {}{}\n", settings.name, settings.path, settings.videoFormat);
}
#pragma endregion

#pragma region DESTRUCTOR
VideoClass::~VideoClass()
{

}
#pragma endregion

#pragma region UPDATE

#pragma endregion