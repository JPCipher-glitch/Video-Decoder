#ifndef VIDEO_CLASS_HPP
#define VIDEO_CLASS_HPP

#include "video/video_stuff.hpp"
#include "video/video_packets.hpp"
#include <string>

class VideoClass final
{
private:
	PacketLoad settings;
	VideoStuff stuff;
public:
	VideoClass() = default;
	VideoClass(PacketLoad* packet);
	~VideoClass();
};

#endif
