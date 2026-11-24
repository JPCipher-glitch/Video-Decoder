#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "video/video_manager.hpp"
#include "server/server_command.hpp"
#include "server/server_debug.hpp"
#include "lifetime.hpp"
#include "error_handler.hpp"

#include <filesystem>

class Engine
{
private:
	std::string path;

	VideoManager videoManager;
	std::shared_ptr<Command> command;
	Lifetime lifetime;
	Debug debug;
	ErrorHandler errorHandler;

	void createCache(const std::string& path);
	bool setDirectoryHidden(const std::filesystem::path& path);
public:
	Engine(const std::string& path);
	~Engine();

	void run();
};

#endif
