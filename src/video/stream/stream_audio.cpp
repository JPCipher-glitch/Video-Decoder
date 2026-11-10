#include "video/stream/stream_audio.hpp"

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

#pragma region CONSTRUCTOR
StreamAudio::StreamAudio(StreamAudioSettings s, ErrorHandler& errorHandler) : settings{ s }, IVidStream{ errorHandler, s.path, s.stream, s.streamIndex }
{
    createAudioCache();
    loadDecoder();
    loadFrame();

    if (invalid) return;

    // 1 - WAV
    if (s.format == ".wav")
    {
        createWavAudioWriter(); // Create the audio writer
        loadWavSwrContext(); // Create the conversion to the wav file
    }
    // 2 - OGG
    else
    {
        // Set OGG by default if the audio format is not valid
        if (s.format != ".ogg")
        {
            errorHandler.showWarning(std::format(L"The audio format \"{}\" is invalid.\nThe format\".ogg\" is now set by default for the audio format", errorHandler.toWString(s.format)), false);
            settings.format = ".ogg";
        }
        loadEncoder(); // Configure the audio settings
        loadFormatContext(); // Open the audio file
        createAudioStream(); // Create the stream & write the headers in the file
        loadOggSwrContext(); // Create the conversion with the encoder
    }
};
#pragma endregion

#pragma region DESTRUCTOR
StreamAudio::~StreamAudio()
{
    // Close file
    closeFile();
    // Remove the cache directory
    fs::path dirPath = std::format("{}/audio", path);
    if (fs::exists(dirPath) && fs::is_directory(dirPath))
        fs::remove_all(dirPath);

    // Cleanup
    av_frame_free(&frame);
    av_frame_free(&convertedFrame);
    av_packet_free(&sendPacket);
    av_packet_free(&receivePacket);
    avcodec_free_context(&decoderContext);
    avcodec_free_context(&encoderContext);
    swr_free(&swrContext);
    av_audio_fifo_free(audioFifo);
};
#pragma endregion

#pragma region DECODER
void StreamAudio::loadDecoder()
{
    // Find a registered decoder
    decoder = avcodec_find_decoder(stream->codecpar->codec_id);

    // Create decoder context
    decoderContext = avcodec_alloc_context3(decoder);
    avcodec_parameters_to_context(decoderContext, stream->codecpar); // Copy codec parameters into codec context

    // Open decoder
    if (avcodec_open2(decoderContext, decoder, nullptr) < 0)
    {
        invalid = true;
        errorHandler.showError(L"Cannot open audio decoder", false);
        return;
    }
}
#pragma endregion

#pragma region CACHE
void StreamAudio::createAudioCache()
{
    // Create the video's cache directory if it don't yet exist
    fs::path audioDirectory = std::format("{}/audio", path);
    if (!fs::exists(audioDirectory))
        fs::create_directories(audioDirectory);
}
#pragma endregion


#pragma region FILE PATH
fs::path StreamAudio::getFilePath(const std::string& format)
{
    AVDictionaryEntry* language = av_dict_get(stream->metadata, "language", nullptr, 0);
	const char* l = language != nullptr ? language->value : "und"; // Use "und" (undefined) if no language is specified
    fs::path filePath = std::format("{}/audio/audio_{}.{}", path, l, format);
    return filePath;
}
#pragma endregion


#pragma region WAV FILE
#pragma region WRITER
void StreamAudio::createWavAudioWriter()
{
    fs::path filePath = getFilePath("wav");
    wavWriter = std::make_unique<WavWriter>(filePath.string(), settings.sampleRate, settings.channels);

    if (settings.channels <= outputLayoutList.size())
        outputLayoutIndex = static_cast<int>(settings.channels) - 1;
    else
    {
        errorHandler.showError(std::format(L"The number of channels for the audio is invalid: {}", settings.channels), false);
        invalid = true;
    }
}
#pragma endregion
#pragma region SWR CONTEXT
void StreamAudio::loadWavSwrContext()
{
    swr_alloc_set_opts2
    (
        &swrContext,
        &outputLayoutList[outputLayoutIndex],
        AV_SAMPLE_FMT_S16,
        settings.sampleRate,
        &decoderContext->ch_layout,
        decoderContext->sample_fmt,
        decoderContext->sample_rate,
        0,
        nullptr
    );

    swr_init(swrContext);
}
#pragma endregion
#pragma endregion

#pragma region OGG FILE
#pragma region ENCODER
void StreamAudio::loadEncoder()
{
    // Find a registered encoder
    encoder = avcodec_find_encoder(AV_CODEC_ID_VORBIS);

    if (encoder == nullptr)
    {
        invalid = true;
        errorHandler.showError(L"The Vorbis encoder was not found", false);
        return;
    }

    // Create encoder context
    encoderContext = avcodec_alloc_context3(encoder);

    // Configure encoder
    encoderContext->sample_rate = static_cast<int>(settings.sampleRate);
    encoderContext->sample_fmt = AV_SAMPLE_FMT_FLTP;
    encoderContext->time_base = AVRational{ 1, encoderContext->sample_rate };

    av_channel_layout_default(&encoderContext->ch_layout, settings.channels);

    // Compression quality: 0 to 10
    encoderContext->flags |= AV_CODEC_FLAG_QSCALE;
    encoderContext->global_quality = settings.quality * FF_QP2LAMBDA;
    encoderContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL; // Disable the restriction of FFMPEG with the Vorbis encoder

    // Open encoder
    if (avcodec_open2(encoderContext, encoder, nullptr) < 0)
    {
        invalid = true;
        errorHandler.showError(L"Cannot open audio encoder", false);
        return;
    }

    // Create audio FIFO
    audioFifo = av_audio_fifo_alloc(encoderContext->sample_fmt, encoderContext->ch_layout.nb_channels, encoderContext->frame_size * 2);

    if (audioFifo == nullptr)
    {
        invalid = true;
        errorHandler.showError(L"Cannot allocate audio FIFO", false);
        return;
    }
}
#pragma endregion
#pragma region AUDIO FORMAT
void StreamAudio::loadFormatContext()
{
    // Load the format context to Write the OGG audio file
    fs::path filePath = getFilePath("ogg");
    int result = avformat_alloc_output_context2(&formatContext, nullptr, "ogg", filePath.string().c_str());

    // Check if the OGG format context was normaly allocated
    if (result < 0)
    {
        invalid = true;
        errorHandler.showError(L"Cannot allocate OGG format context", false);
        return;
    }

    // Open the OGG file
    result = avio_open(&formatContext->pb, filePath.string().c_str(), AVIO_FLAG_WRITE);
    if (result < 0)
    {
        invalid = true;
        errorHandler.showError(L"Cannot open OGG file", false);
        return;
    }
}
#pragma endregion
#pragma region STREAM
void StreamAudio::createAudioStream()
{
    // Create the audio stream with the format context for the OGG file
    outputStream = avformat_new_stream(formatContext, nullptr);
    if (outputStream == nullptr)
    {
        invalid = true;
        errorHandler.showError(L"Cannot create audio stream", false);
        return;
    }

    // Set the settings from the encoder into the stream
    avcodec_parameters_from_context(outputStream->codecpar, encoderContext);
    outputStream->time_base = encoderContext->time_base;

    // Write the headers of the OGG file
    if (!(formatContext->oformat->flags & AVFMT_NOFILE))
        avformat_write_header(formatContext, nullptr);
}
#pragma endregion
#pragma region SWRCONTEXT
void StreamAudio::loadOggSwrContext()
{
    swr_alloc_set_opts2
    (
        &swrContext,
        &encoderContext->ch_layout,
        encoderContext->sample_fmt,
        encoderContext->sample_rate,
        &decoderContext->ch_layout,
        decoderContext->sample_fmt,
        decoderContext->sample_rate,
        0,
        nullptr
    );

    swr_init(swrContext);
}
#pragma endregion

#pragma endregion

#pragma region FRAME
void StreamAudio::loadFrame()
{
    if (invalid) return;

    // Allocate the AVPacket & AVFrame
    sendPacket = av_packet_alloc();
    receivePacket = av_packet_alloc();
    frame = av_frame_alloc();
    convertedFrame = av_frame_alloc();
}

void StreamAudio::receiveFrame(AVFormatContext*& formatContext)
{
    if (invalid) return;

    av_seek_frame(formatContext, -1, 0, AVSEEK_FLAG_BACKWARD); // Reset the reader cursor into the start
    avcodec_flush_buffers(decoderContext); // Wipe the last buffers before reading another sample

    // Decode frames
    while (av_read_frame(formatContext, sendPacket) >= 0)
    {
        // Get the audio stream
        if (sendPacket->stream_index == streamIndex)
        {
            // Send compressed packet to decoder
            avcodec_send_packet(decoderContext, sendPacket);

            // Receive decoded frames
            while (true)
            {
                // Receive the decoded data frame
                int result = avcodec_receive_frame(decoderContext, frame);
                if (result == AVERROR(EAGAIN) || result == AVERROR_EOF)
                    break;

                // Create the audio file
                saveAsFile();

                // Wipe the packet to reusing it
                av_frame_unref(frame);
            }
        }
        av_packet_unref(sendPacket); // Wipe the packet to reusing it
    }

    // Close the audio file to complete it
    closeFile();
}
#pragma endregion

#pragma region FILE
#pragma region WAV FILE
void StreamAudio::saveWavFile()
{
    int64_t nbSamples = av_rescale_rnd
    (
        swr_get_delay(swrContext, decoderContext->sample_rate) + frame->nb_samples,
        44100,
        decoderContext->sample_rate,
        AVRounding::AV_ROUND_UP
    );
    av_frame_unref(convertedFrame);

    convertedFrame->format = AV_SAMPLE_FMT_S16;
    convertedFrame->sample_rate = 44100;
    av_channel_layout_copy(&convertedFrame->ch_layout, &outputLayoutList[outputLayoutIndex]);
    convertedFrame->nb_samples = static_cast<int>(nbSamples);

    // Prepare the PCM buffer
    if (av_frame_get_buffer(convertedFrame, 0) < 0)
        throw std::runtime_error("ERROR: Cannot allocate PCM buffer");

    // Convert the PCM buffer into audio samples
    int samples = swr_convert
    (
        swrContext,
        convertedFrame->data,
        convertedFrame->nb_samples,
        const_cast<const uint8_t**>(frame->extended_data),
        frame->nb_samples
    );
    if (samples < 0)
        throw std::runtime_error("Cannot convert audio samples");

    if (samples == 0)
        return;

    // PCM size
    uint32_t pcmSize = static_cast<uint32_t>(samples * settings.channels * sizeof(int16_t));
    wavWriter->writeSamples(convertedFrame->data[0], pcmSize);
}
void StreamAudio::closeWavFile()
{
    wavWriter->close();
}
#pragma endregion
#pragma region OGG FILE
void StreamAudio::writeEncodedPackets()
{
    // Receive all the encoder's frames
    while (true)
    {
        // Receive the decoded data frame
        int result = avcodec_receive_packet(encoderContext, receivePacket);
        if (result == AVERROR(EAGAIN) || result == AVERROR_EOF)
            break;

        // Convert the time base of the packet
        av_packet_rescale_ts(receivePacket, encoderContext->time_base, outputStream->time_base);
        // Set the output stream index in the packet
        receivePacket->stream_index = outputStream->index;

        // Write the audio sample into the OGG file
        if (av_interleaved_write_frame(formatContext, receivePacket) < 0)
        {
            invalid = true;
            errorHandler.showError(L"Cannot write OGG packet", false);
            return;
        }

        av_packet_unref(receivePacket); // Wipe the packet to reusing it
    }
}
void StreamAudio::encodeCompleteFrames()
{
    av_frame_unref(convertedFrame);

    convertedFrame->format = encoderContext->sample_fmt;
    convertedFrame->sample_rate = encoderContext->sample_rate;
    av_channel_layout_copy(&convertedFrame->ch_layout, &encoderContext->ch_layout);
    convertedFrame->nb_samples = encoderContext->frame_size;

    if (av_frame_get_buffer(convertedFrame, 0) < 0)
    {
        invalid = true;
        errorHandler.showError(L"Cannot allocate Vorbis frame", false);
        return;
    }

    // Read samples from FIFO
    if (av_audio_fifo_read(audioFifo, reinterpret_cast<void**>(convertedFrame->extended_data), encoderContext->frame_size) != encoderContext->frame_size)
    {
        invalid = true;
        errorHandler.showError(L"Cannot read samples from audio FIFO", false);
        return;
    }

    // Set timestamps
    convertedFrame->pts = nextOggPts;
    nextOggPts += convertedFrame->nb_samples;

    // Send frame to encoder
    int result = avcodec_send_frame(encoderContext, convertedFrame);
    if (result == AVERROR(EAGAIN))
    {
        writeEncodedPackets();
        result = avcodec_send_frame(encoderContext, convertedFrame);
    }

    if (result < 0)
    {
        invalid = true;
        errorHandler.showError(L"Failed to send frame to Vorbis encoder", false);
        return;
    }

    writeEncodedPackets();
}
void StreamAudio::saveOggFile()
{
    int64_t nbSamples = av_rescale_rnd
    (
        swr_get_delay(swrContext, decoderContext->sample_rate) + frame->nb_samples,
        encoderContext->sample_rate,
        decoderContext->sample_rate,
        AV_ROUND_UP
    );
    av_frame_unref(convertedFrame);

    convertedFrame->format = encoderContext->sample_fmt;
    convertedFrame->sample_rate = encoderContext->sample_rate;
    av_channel_layout_copy(&convertedFrame->ch_layout, &encoderContext->ch_layout);
    convertedFrame->nb_samples = static_cast<int>(nbSamples);

    // Prepare the conversion Buffer
    if (av_frame_get_buffer(convertedFrame, 0) < 0)
    {
        invalid = true;
        errorHandler.showError(L"Cannot allocate OGG audio buffer", false);
        return;
    }

    // Convert audio samples
    int samples = swr_convert
    (
        swrContext,
        convertedFrame->data,
        convertedFrame->nb_samples,
        const_cast<const uint8_t**>(frame->extended_data),
        frame->nb_samples
    );
    if (samples < 0)
    {
        invalid = true;
        errorHandler.showError(L"Cannot convert audio samples", false);
        return;
    }

    if (samples == 0)
        return;

    // Set the number of samples
    convertedFrame->nb_samples = samples;

    // Add converted samples into FIFO
    if (av_audio_fifo_realloc(audioFifo, av_audio_fifo_size(audioFifo) + samples) < 0)
    {
        invalid = true;
        errorHandler.showError(L"Cannot resize audio FIFO", false);
        return;
    }

    if (av_audio_fifo_write(audioFifo, reinterpret_cast<void**>(convertedFrame->extended_data), samples) < samples)
    {
        invalid = true;
        errorHandler.showError(L"Cannot write samples into audio FIFO", false);
        return;
    }

    // Encode complete frames
    while (av_audio_fifo_size(audioFifo) >= encoderContext->frame_size)
    {
        encodeCompleteFrames();
    }
}
void StreamAudio::closeOggFile()
{
    // Flush the sampler to get the last samples
    int64_t delay = swr_get_delay(swrContext, decoderContext->sample_rate);
    if (delay > 0)
        saveOggFile();

    // Flush encoder
    avcodec_send_frame(encoderContext, nullptr);
    writeEncodedPackets();

    // Write OGG trailer
    av_write_trailer(formatContext);

    // Close the OGG file
    if (!(formatContext->oformat->flags & AVFMT_NOFILE))
        avio_closep(&formatContext->pb);
}
#pragma endregion
#pragma region BASE FILE
void StreamAudio::saveAsFile()
{
    if (settings.format == ".wav")
        saveWavFile();
    else if (settings.format == ".ogg")
        saveOggFile();
}
#pragma endregion
#pragma region CLOSE FILE
void StreamAudio::closeFile()
{
    if (isFileClosed) return;

    if (settings.format == ".wav")
        closeWavFile();
    else if (settings.format == ".ogg")
        closeOggFile();

	isFileClosed = true;
}
#pragma endregion
#pragma endregion