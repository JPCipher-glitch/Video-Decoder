#ifndef VIDEO_CLASS_HPP
#define VIDEO_CLASS_HPP

#include "video/video_stuff.hpp"
#include "video/video_packets.hpp"

#include <mutex>
#include <string>

class VideoClass final
{
private:
	std::mutex mutex;

	std::string path;
	PacketLoad settings;
	VideoStuff stuff;

	void createCacheFolder();

	void loadContext();
	void loadStreams();
	void displayInfo();
public:
	VideoClass() = default;
	VideoClass(const std::string& path, PacketLoad* packet);
	~VideoClass();
};

#endif
