#ifndef INSTANCE_MANAGER_HPP
#define INSTANCE_MANAGER_HPP

#include <boost/interprocess/sync/file_lock.hpp>
#include <memory>

class InstanceManager
{
private:
    bool isRunning = false;
    std::unique_ptr<boost::interprocess::file_lock> lock;
public:
    InstanceManager();
    ~InstanceManager();

    bool isAnotherInstanceRunning();
};

#endif