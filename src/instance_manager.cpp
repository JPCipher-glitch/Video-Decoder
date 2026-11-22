#include "instance_manager.hpp"
#include <filesystem>
#include <fstream>
#include <format>

#pragma region CONSTRUCTOR
InstanceManager::InstanceManager()
{
    // Get the instance type of the app (between debug & release)
#ifdef _DEBUG
    std::string instanceType = "Debug";
#else
    std::string instanceType = "Release";
#endif
    // Create the empty file if it doesn't exist
    std::filesystem::path lockPath = (std::filesystem::temp_directory_path() / std::format("VideoDecoder_{}.lock", instanceType)).string();
    if (!std::filesystem::exists(lockPath))
        std::ofstream file(std::filesystem::absolute(lockPath));

    // Create the lock to shutdown the other instances of this program if one is currently executing
    lock = std::make_unique<boost::interprocess::file_lock>(lockPath.c_str());
    isRunning = lock->try_lock(); // Try to acquire the lock
}
#pragma endregion

#pragma region DESTRUCTOR
InstanceManager::~InstanceManager()
{
    // Release the mutex when the program is closing
    if (isRunning)
        lock->unlock();
}
#pragma endregion

#pragma region ANOTHER_INSTANCE
bool InstanceManager::isAnotherInstanceRunning()
{
    return !isRunning;
}
#pragma endregion
