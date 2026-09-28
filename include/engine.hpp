#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "video/video_manager.hpp"
#include "command.hpp"
#include "lifetime.hpp"
#include "debug.hpp"

class Engine
{
private:
	VideoManager videoManager;
	Command command;
	Lifetime lifetime;
	Debug debug;
public:
	Engine();
	~Engine();

	void run();
};

#endif
