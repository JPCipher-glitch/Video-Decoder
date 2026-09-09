#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "video_manager.hpp"
#include "lifetime.hpp"

class Engine
{
private:
	VideoManager videoManager;
	// Command
	Lifetime lifetime;
	// Debug
public:
	Engine();
	~Engine();

	void run();
};

#endif
