#ifndef VIDEO_MANAGER_HPP
#define VIDEO_MANAGER_HPP

#include "observer.hpp"
#include "video/video_class.hpp"
#include "serializer/command_type.hpp"

#include <string>
#include <unordered_map>
#include <functional>

class Lifetime;
struct IPacket;

class VideoManager : public IObserver
{
private:
	std::string path;
	Lifetime& lifetime;
	std::unordered_map<std::string, std::unique_ptr<VideoClass>> videoMap;
	std::unordered_map<COMMAND_TYPE, std::function<void(IPacket*)>> callMap;

	void loadVideo(IPacket* packet);
public:
	VideoManager(const std::string& path, Lifetime& lifetime);
	~VideoManager();

	void setPath(const std::string& path);

	void call(IPacket* packet) override;
	void update();
};

#endif
