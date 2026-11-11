#ifndef VIDEO_LIFETIME_HPP
#define VIDEO_LIFETIME_HPP

#include <atomic>

struct VideoLifetime final
{
	std::atomic<bool> isDone = false;
};

#endif
