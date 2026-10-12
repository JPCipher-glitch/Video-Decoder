#ifndef VIDEO_CLASS_HPP
#define VIDEO_CLASS_HPP

#include "video/video_stuff.hpp"
#include "video/video_packets.hpp"

#include <unordered_map>
#include <mutex>
#include <string>
#include <functional>

class VideoClass final
{
private:
	std::mutex mutex;

	std::string path;
	PacketLoad settings;
	VideoStuff stuff;

	bool initialized = false;

	std::unordered_map<AVMediaType, std::function<std::unique_ptr<IStream>(AVStream*, int)>> loadStreamMap;

	void createCacheFolder();

	void loadContext();
	void loadStreams();
	void displayInfo();
public:
	VideoClass() = default;
	VideoClass(const std::string& path, PacketLoad* packet);
	~VideoClass();

	void load();
	void update();
};

#endif
