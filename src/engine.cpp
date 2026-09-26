#include "engine.hpp"
#include "video/video_manager.hpp"

Engine::Engine() : videoManager(lifetime)
{
	command.attach(&videoManager);
}

Engine::~Engine()
{

}

void Engine::run()
{
	while (lifetime.isAlive())
	{
		videoManager.update();
		command.update();
		lifetime.update();
	}
}
