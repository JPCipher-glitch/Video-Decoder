#ifndef VIDEO_CLASS_HPP
#define VIDEO_CLASS_HPP

#include "video/video_stuff.hpp"
#include "video/video_packets.hpp"
#include "error_handler.hpp"

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
	ErrorHandler& errorHandler;

	bool initialized = false;
	bool valid = true;

	std::unordered_map<AVMediaType, std::function<std::unique_ptr<IVidStream>(AVStream*, int)>> loadStreamMap;

	void createCacheFolder();

	void loadContext();
	void loadStreams();
	void displayInfo();
public:
	VideoClass(ErrorHandler& errorHandler);
	VideoClass(const std::string& path, PacketLoad* packet, ErrorHandler& errorHandler);
	~VideoClass();

	bool isValid();

	void load();
	void update();
};

#endif
