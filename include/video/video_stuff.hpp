#ifndef VIDEO_STUFF_HPP
#define VIDEO_STUFF_HPP

#include "video/stream/stream.hpp"

#include <unordered_map>
#include <unordered_set>
#include <memory>

struct VideoStuff final
{
	// Video Main Context
	AVFormatContext* formatContext = nullptr;

	// Video informations
	std::unordered_map<AVMediaType, std::unique_ptr<IStream>> videoStreamMap;
	std::unordered_set<AVMediaType> availableStreamType{ AVMediaType::AVMEDIA_TYPE_VIDEO, AVMediaType::AVMEDIA_TYPE_AUDIO, AVMediaType::AVMEDIA_TYPE_SUBTITLE };
	AVCodecParameters* codecParameters = nullptr;
};

#endif
