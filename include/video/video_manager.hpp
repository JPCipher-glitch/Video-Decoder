#ifndef VIDEO_MANAGER_HPP
#define VIDEO_MANAGER_HPP

#include "observer.hpp"

#include <string>
#include <unordered_map>

class Lifetime;
struct IPacket;

class VideoManager : public IObserver
{
private:
	Lifetime& lifetime;
	std::unordered_map<std::string, int> videoMap;

	void loadVideo(const std::string& name, const std::string& path, IPacket* packet);
public:
	VideoManager(Lifetime& lifetime);
	~VideoManager();

	void call(IPacket* packet) override;
	void update();
};

#endif
