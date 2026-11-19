#include "video/stream/stream_video.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

#pragma region CONSTRUCTOR
StreamVideo::StreamVideo(StreamFrameSettings s, ErrorHandler& errorHandler, VideoLifetime& videoLifetime, LoadingHandler& loadingHandler) : settings{ s }, IVidStream{ errorHandler, videoLifetime, loadingHandler, s.path, s.stream, s.streamIndex }
{
    loadingWeight = 0.3;

    createFramesCache();
    loadDecoder();
    loadEncoder();
    loadFrame();
    loadFilter();
    loadSwsContext();
    createExportFrame();
}
#pragma endregion

#pragma region DESTRUCTOR
StreamVideo::~StreamVideo()
{
    // Cleanup
    av_frame_free(&frame);
    av_frame_free(&filteredFrame);
    av_packet_free(&sendPacket);
    av_packet_free(&receivePacket);
    avcodec_free_context(&decoderContext);
    avcodec_free_context(&encoderContext);
	// Filter cleanup
    if (filterData.filterGraph != nullptr)
    {
        avfilter_graph_free(&filterData.filterGraph);
        filterData.bufferSrcContext = nullptr;
        filterData.bufferSinkContext = nullptr;
    }
}
#pragma endregion

#pragma region DECODER
void StreamVideo::loadDecoder()
{
    if (invalid) return;

    // Find a registered decoder
    if (stream->codecpar->codec_id == AV_CODEC_ID_AV1)
		decoder = avcodec_find_decoder_by_name("libdav1d"); // Use libdav1d decoder for AV1 files
    else
        decoder = avcodec_find_decoder(stream->codecpar->codec_id);

    // Create decoder context
    decoderContext = avcodec_alloc_context3(decoder);
    avcodec_parameters_to_context(decoderContext, stream->codecpar); // Copy codec parameters into codec context

    // Open decoder
    int result = avcodec_open2(decoderContext, decoder, nullptr);
    if (result < 0)
    {
        char errorBuffer[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(result, errorBuffer, sizeof(errorBuffer));

        std::cerr << "Decoder initialization failed: " << errorBuffer << std::endl;

        invalid = true;
        errorHandler.showError(std::format("Cannot open video-stream decoder:\n{}", errorBuffer), false);
        return;
    }
}
#pragma endregion

#pragma region ENCODER
void StreamVideo::loadEncoder()
{
    if (invalid) return;

    // Find the encoder context
    encoder = avcodec_find_encoder(AVCodecID::AV_CODEC_ID_PNG);

    // Create the PNG encoder
    encoderContext = avcodec_alloc_context3(encoder);

    // Calculate the FPS
    double fps = av_q2d(stream->avg_frame_rate);
	uint8_t roundedFps = static_cast<uint8_t>(std::round(fps));
    uint8_t finalFps = std::min(roundedFps, settings.fps); // Use the minimum between the rounded FPS and the user-defined FPS
    settings.fps = std::max(finalFps, uint8_t(1U));

    // Get the settings
    encoderContext->width = settings.width;
    encoderContext->height = settings.height;
    encoderContext->pix_fmt = AV_PIX_FMT_RGB24;
    encoderContext->time_base = { 1, settings.fps };

    // Open encoder
    avcodec_open2(encoderContext, encoder, nullptr);
}
#pragma endregion

#pragma region FILTER
bool StreamVideo::loadFilter()
{
    if (invalid) return false;

	const AVFilter* bufferSrc = avfilter_get_by_name("buffer"); // Get the filter where the frame will be sent to
	const AVFilter* bufferSink = avfilter_get_by_name("buffersink"); // Get the filter where the frame will be received from

	// Create the filter graph
    filterData.filterGraph = avfilter_graph_alloc();
    if (filterData.filterGraph == nullptr)
        return false;

    const AVRational timeBase = stream->time_base;
    const AVRational sampleAspectRatio = decoderContext->sample_aspect_ratio;

	// Define the arguments for the buffer source filter
    char args[512];
    snprintf(args, sizeof(args), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d", decoderContext->width, decoderContext->height, decoderContext->pix_fmt, timeBase.num, timeBase.den, sampleAspectRatio.num, sampleAspectRatio.den);

	// Create the instances of the buffer source and buffer sink filters
    avfilter_graph_create_filter(&filterData.bufferSrcContext, bufferSrc, "in", args, nullptr, filterData.filterGraph);
    avfilter_graph_create_filter(&filterData.bufferSinkContext, bufferSink, "out", nullptr, nullptr, filterData.filterGraph);

	// Create the FPS filter to limit the frame rate
    AVFilterContext* fpsFilter = nullptr;
    std::string fpsArgs = std::format("fps={}", settings.fps);
    avfilter_graph_create_filter(&fpsFilter, avfilter_get_by_name("fps"), "fps", fpsArgs.c_str(), nullptr, filterData.filterGraph);

	// Link the filters together
    avfilter_link(filterData.bufferSrcContext, 0, fpsFilter, 0);
    avfilter_link(fpsFilter, 0, filterData.bufferSinkContext, 0);

	// Configure the filter graph
    if (avfilter_graph_config(filterData.filterGraph, nullptr) < 0)
    {
        invalid = true;
        errorHandler.showError("Cannot configure the filter graph", false);
        return false;
    }

	std::cout << std::format("FPS: {}", settings.fps) << std::endl;

    return true;
}
#pragma endregion


#pragma region SWS CONTEXT
void StreamVideo::loadSwsContext()
{
    if (invalid) return;

    // Create the context to convert the frame into a RGB frame
    swsContext = sws_getContext
    (
        decoderContext->width,
        decoderContext->height,
        decoderContext->pix_fmt,

        encoderContext->width,
        encoderContext->height,
        AV_PIX_FMT_RGB24,

        SWS_POINT,
        nullptr,
        nullptr,
        nullptr
    );
}
#pragma endregion

#pragma region EXPORT FRAME
void StreamVideo::createExportFrame()
{
    if (invalid) return;

    // Create the RGB frame
    exportFrame = av_frame_alloc();

    exportFrame->format = AV_PIX_FMT_RGB24;
    exportFrame->width = encoderContext->width;
    exportFrame->height = encoderContext->height;
    if (av_frame_get_buffer(exportFrame, 0) < 0)
    {
        invalid = true;
        errorHandler.showError("Cannot allocate export frame buffer", false);
        return;
    }
}
#pragma endregion


#pragma region CACHE
void StreamVideo::createFramesCache()
{
    // Create the video's cache directory if it don't yet exist
    fs::path audioDirectory = std::format("{}/frames", path);
    if (!fs::exists(audioDirectory))
        fs::create_directories(audioDirectory);
}
#pragma endregion


#pragma region FILE PATH
fs::path StreamVideo::getFilePath(const std::string& format)
{
    fs::path filePath = std::format("{}/frames/frame_{:06}{}", path, frameIndex, format);
    return filePath;
}
#pragma endregion

#pragma region LOADING
void StreamVideo::addProgress()
{
    // Calculate the progress wait
    double totalWeight = loadingWeight / loadingHandler.loadingWeightRatio * 100.0;
    double weight = totalWeight / static_cast<double>(preloadingFrameLimit);
    loadingHandler.addProgress(weight);
}
#pragma endregion

#pragma region FRAME
void StreamVideo::loadFrame()
{
    if (invalid) return;
    // Allocate the AVPacket & AVFrame
    sendPacket = av_packet_alloc();
    frame = av_frame_alloc();
    receivePacket = av_packet_alloc();
    filteredFrame = av_frame_alloc();
}

bool StreamVideo::writeFrame(AVFormatContext*& formatContext)
{
    if (invalid) return false;

    // Get the audio stream
    if (sendPacket->stream_index == streamIndex)
    {
        // Send compressed packet to decoder
        if (int result = avcodec_send_packet(decoderContext, sendPacket) < 0)
        {
            invalid = true;

            char errorBuffer[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(result, errorBuffer, sizeof(errorBuffer));

            errorHandler.showError(std::format("Video Stream - Decoder error: {}", errorBuffer), false);
            std::cerr << "Video Stream - Decoder error: " << errorBuffer << std::endl;
            return false;
        }

        // Receive decoded frames
        while (!videoLifetime.isDone)
        {
            int a;
            // Receive the decoded data frame
            int result = avcodec_receive_frame(decoderContext, frame);
            if (result == AVERROR(EAGAIN) || result == AVERROR_EOF)
                return true;

			// Send the frame to the filter graph
            result = av_buffersrc_add_frame_flags(filterData.bufferSrcContext, frame, AV_BUFFERSRC_FLAG_KEEP_REF);
            if (result == AVERROR(EAGAIN) || result == AVERROR_EOF)
                return true;

			// Check if the frame has been sent to the filter graph
            if (result < 0)
            {
                invalid = true;
                errorHandler.showError("Cannot send frame to filter graph in Video Stream", false);
                return false;
            }

			// Get the filtered frame from the filter graph
            while (av_buffersink_get_frame(filterData.bufferSinkContext, filteredFrame) >= 0)
            {
                // Create the audio file
                saveAsFile();
                // Add loading progress
                addProgress();

                // Wipe the packet to reusing it
                av_frame_unref(filteredFrame);
            }

            // Wipe the packet to reusing it
            av_frame_unref(frame);
        }
    }
    av_packet_unref(sendPacket); // Wipe the packet to reusing it
    return true;
}

void StreamVideo::receiveFrame(AVFormatContext*& formatContext)
{
    if (invalid) return;

    loadingHandler.setStatus("Loading Frames");

    av_seek_frame(formatContext, streamIndex, 0, AVSEEK_FLAG_BACKWARD); // Reset the reader cursor into the start
    avcodec_flush_buffers(decoderContext); // Wipe the last buffers before reading another sample

    // Decode frames
    while (av_read_frame(formatContext, sendPacket) >= 0 && !videoLifetime.isDone && !invalid)
    {
        if (!writeFrame(formatContext) || frameIndex >= 10)
            break;
    }

	// Flush the decoder
    avcodec_send_packet(decoderContext, nullptr);
	// Flush the filter decoder
    av_buffersrc_add_frame_flags(filterData.bufferSrcContext, nullptr, 0);

	// Get the lost frames from the filter graph
    while (av_buffersink_get_frame(filterData.bufferSinkContext, filteredFrame) >= 0)
    {
        // Create the audio file
        saveAsFile();
        // Add loading progress
        addProgress();

        // Wipe the packet to reusing it
        av_frame_unref(filteredFrame);
    }
}
#pragma endregion

#pragma region FILE
void StreamVideo::saveAsFile()
{
    // Rescale the export filteredFrame with the sws ctx
    sws_scale
    (
        swsContext,

        filteredFrame->data,
        filteredFrame->linesize,

        0,
        filteredFrame->height,

        exportFrame->data,
        exportFrame->linesize
    );

    // Create the frame packet
    int result = avcodec_send_frame(encoderContext, exportFrame);
    if (result >= 0)
        result = avcodec_receive_packet(encoderContext, receivePacket);
    else
    {
        std::cerr << "Failed to receive PNG packet!" << std::endl;
        return;
    }

    // Save the frame into an image
    std::ofstream file(getFilePath(settings.format), std::ios::binary);
    file.write(reinterpret_cast<const char*>(receivePacket->data), receivePacket->size);
    file.close();

    // Wipe the packet to reusing it
    av_packet_unref(receivePacket);
    frameIndex++;
}
#pragma endregion
#pragma region CLOSE FILE
void StreamVideo::closeFile()
{

}
#pragma endregion
#pragma endregion
