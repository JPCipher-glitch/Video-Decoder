#ifndef VIDEO_MANAGER_HPP
#define VIDEO_MANAGER_HPP

#include "observer.hpp"
#include "lifetime.hpp"

#include <string>
#include <unordered_map>

class VideoManager : public IObserver
{
private:
	Lifetime* lifetime = nullptr;
	std::unordered_map<std::string, int> videoMap;
public:
	VideoManager(Lifetime* lifetime);
	~VideoManager();

	void call() override;
	void update();
};

#endif
