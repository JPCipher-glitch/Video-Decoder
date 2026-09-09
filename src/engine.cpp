#include "engine.hpp"
#include "video_manager.hpp"

Engine::Engine() : videoManager(&lifetime)
{
	
}

Engine::~Engine()
{

}

void Engine::run()
{
	while (lifetime.isAlive())
	{
		videoManager.update();
		lifetime.update();
	}
}
