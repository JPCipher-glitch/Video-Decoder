#include "video/video_manager.hpp"
#include "video/video_packets.hpp"
#include "video/video_class.hpp"

#include "lifetime.hpp"

VideoManager::VideoManager(Lifetime& lifetime) : lifetime(lifetime)
{

}

VideoManager::~VideoManager()
{

}

void VideoManager::call(IPacket* packet)
{

}

void VideoManager::update()
{

}