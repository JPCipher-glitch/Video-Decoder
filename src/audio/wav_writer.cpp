#include "audio/wav_writer.hpp"
#include <iostream>
#include <filesystem>

#pragma region CONSTRUCTOR
WavWriter::WavWriter(const std::string& path, uint32_t sampleRate, uint16_t channels)
{
    open(path, sampleRate, channels);
}
#pragma endregion

#pragma region DESTRUCTOR
WavWriter::~WavWriter()
{
    close();
}
#pragma endregion

#pragma region SAMPLES
void WavWriter::writeSamples(const uint8_t* data, uint32_t size)
{
    file.write(reinterpret_cast<const char*>(data), size);
}
#pragma endregion

#pragma region OPEN
void WavWriter::open(const std::string& path, uint32_t sampleRate, uint16_t channels)
{
    // Don't open if the file it is already open
    if (file.is_open())
        return;

    file = std::ofstream(path, std::ios::binary);

    // Write the file headers
    file << "RIFF----WAVEfmt ";

    writeWord(file, 16, 4); // Format block size
    writeWord(file, 1, 2); // PCM - integer samples
    writeWord(file, channels, 2);  // Channels (mono/stereo file)
    writeWord(file, sampleRate, 4);  // Samples per second (Hz)
    writeWord(file, sampleRate * channels * 2, 4);  // Sample Rate per byte
    writeWord(file, channels * 2, 2);  // Data block size
    writeWord(file, 16, 2);  // Bits per sample

    dataChunkPos = file.tellp();
    file << "data----"; // Chunk size
}
#pragma endregion

#pragma region CLOSE
void WavWriter::close()
{
    // Don't close if the file don't exist anymore
    if (!file.is_open())
        return;

    size_t fileLength = file.tellp();

    // Fix the data chunk header to contain the data size
    file.seekp(dataChunkPos + 4);
    writeWord(file, fileLength - dataChunkPos + 8);

    // Fix the file header to contain the proper RIFF chunk size, which is (file size - 8) bytes
    file.seekp(0 + 4);
    writeWord(file, fileLength - 8, 4);

    // Close the file
    file.close();
}
#pragma endregion
