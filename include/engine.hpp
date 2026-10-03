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
	std::shared_ptr<Command> command;
	Lifetime lifetime;
	Debug debug;
public:
	Engine();
	~Engine();

	void run();
};

#endif
