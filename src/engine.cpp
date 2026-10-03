#include "engine.hpp"
#include "video/video_manager.hpp"

Engine::Engine() : videoManager(lifetime)
{
	command = std::make_shared<Command>();
	command->accept();
	command->run();
	command->attach(&videoManager);

	debug.run();
}

Engine::~Engine()
{

}

void Engine::run()
{
	while (lifetime.isAlive())
	{
		videoManager.update();
		command->update();
		lifetime.update();
	}
}
