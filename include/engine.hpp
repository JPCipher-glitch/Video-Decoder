#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "video/video_manager.hpp"
#include "command.hpp"
#include "lifetime.hpp"

class Engine
{
private:
	VideoManager videoManager;
	Command command;
	Lifetime lifetime;
	// Debug
public:
	Engine();
	~Engine();

	void run();
};

#endif
