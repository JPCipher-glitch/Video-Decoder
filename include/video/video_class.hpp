#ifndef VIDEO_CLASS_HPP
#define VIDEO_CLASS_HPP

#include "video/video_stuff.hpp"
#include <string>

struct VideoSettings final
{
	std::string name;
	std::string path;

	std::string frameFormat = ".jpg";
	std::string audioFormat = ".ogg";

	uint16_t width = 1920u;
	uint16_t height = 1080u;
	float fps = 60.f;
};

class VideoClass final
{
private:
	VideoSettings settings;
	VideoStuff stuff;
public:
	VideoClass(const std::string& name, const std::string& path, IPacket* packet);
	~VideoClass();
};

#endif
