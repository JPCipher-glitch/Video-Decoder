#include "engine.hpp"
#include "video/video_manager.hpp"

#include <QApplication>
#include <format>

namespace fs = std::filesystem;

bool Engine::setDirectoryHidden(const std::filesystem::path& path)
{
	// Get the directory attributes 
	DWORD attributes = GetFileAttributesW(path.c_str());
	if (attributes == INVALID_FILE_ATTRIBUTES) // Check if the attributes are valids
		return false;

	// Enable the "Hidden" attribute to the directory
	attributes |= FILE_ATTRIBUTE_HIDDEN;

	// Return the modified attributes
	return SetFileAttributesW(path.c_str(), attributes);
}

void Engine::createCache(const std::string& path)
{
	fs::path cacheDirectory = std::format("{}/.cache", path);
	if (!fs::exists(cacheDirectory))
		fs::create_directories(cacheDirectory);

	// Hide the cache directory
	setDirectoryHidden(cacheDirectory);
}

Engine::Engine(const std::string& p) : path(p), videoManager(path, lifetime, errorHandler), debug(lifetime, errorHandler)
{
	// Set the cache path
	if (path.length() <= 0)
		path = "placeholders";

	// Check if the folder exist to making the cache, else shutdown the engine
	fs::path cacheDirectory = path;
	if (!fs::exists(cacheDirectory))
	{
		std::string message = std::format("This folder is not valid for saving the video cache:\n\"{}\"\nThe engine will shutdown now.", path);
		errorHandler.showError(message, true);
		errorHandler.update();
		lifetime.destroy();
		return;
	}

	// Set the cache path to the video manager
	videoManager.setPath(path);

	// Create cache folder
	createCache(path);

	// Create command server
	command = std::make_shared<Command>(errorHandler);
	command->accept();
	command->run();
	command->attach(&videoManager);

	// Wait a little bit to make sure the server is running before creating the debug client
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// Create debug client
	debug.run();
}

Engine::~Engine()
{
	QApplication::closeAllWindows(); // Close all the QT widgets
}

void Engine::run()
{
	while (lifetime.isAlive())
	{
		QApplication::processEvents(); // Process the QT events into the loop

		videoManager.update();
		command->update();
		lifetime.update();
		errorHandler.update();
	}
}
