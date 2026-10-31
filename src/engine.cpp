#include "engine.hpp"
#include "video/video_manager.hpp"
#include "Windows.h"

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

Engine::Engine(const std::string& p) : path(p), videoManager(path, lifetime), debug(lifetime, errorHandler)
{
	// Set the cache path
	if (path.length() <= 0)
		path = "resources";

	// Check if the folder exist to making the cache, else shutdown the engine
	fs::path cacheDirectory = path;
	if (!fs::exists(cacheDirectory))
	{
		std::wstring message = std::format(L"This folder is not valid for saving the video cache: ");
		errorHandler.showError(message, true);
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

	// Create debug client
	debug.run();
}

Engine::~Engine()
{
	// Remove the cache directory
	fs::path dirPath = std::format("{}/.cache", path);
	if (fs::exists(dirPath) && fs::is_directory(dirPath))
		fs::remove_all(dirPath);
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
