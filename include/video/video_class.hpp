#ifndef VIDEO_CLASS_HPP
#define VIDEO_CLASS_HPP

#include "video/video_stuff.hpp"
#include "video/video_packets.hpp"
#include "video/video_lifetime.hpp"
#include "error_handler.hpp"

#include <atomic>
#include <thread>
#include <mutex>
#include <functional>
#include <unordered_map>
#include <string>

class VideoClass final
{
private:
	std::mutex mutex;

	std::jthread loadingTask;
	std::atomic<bool> isLoaded = false;
	std::atomic<bool> isLoading = false;

	std::string path;
	PacketLoad settings;
	VideoStuff stuff;
	VideoLifetime videoLifetime;
	ErrorHandler& errorHandler;

	double streamTimeStart = 0.0;

	bool islog = false;
	bool initialized = false;
	bool valid = true;

	std::unordered_map<AVMediaType, std::function<std::unique_ptr<IVidStream>(AVStream*, int)>> loadStreamMap;

	void createCacheFolder();

	void loadAssets();
	void loadContext();
	void loadStreams();
	void displayInfo();
public:
	VideoClass(const std::string& path, PacketLoad* packet, ErrorHandler& errorHandler);
	~VideoClass();

	bool isValid();
	void requestStop();
	void join();

	void load();
	void update();
};

#endif
