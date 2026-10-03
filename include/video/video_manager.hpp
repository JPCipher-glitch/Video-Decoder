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
	Lifetime& lifetime;
	std::unordered_map<std::string, VideoClass> videoMap;
	std::unordered_map<COMMAND_TYPE, std::function<void(IPacket*)>> callMap;

	void loadVideo(IPacket* packet);
public:
	VideoManager(Lifetime& lifetime);
	~VideoManager();

	void call(IPacket* packet) override;
	void update();
};

#endif
