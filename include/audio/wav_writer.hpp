#ifndef WAV_WRITTER_HPP
#define WAV_WRITTER_HPP

#include <fstream>

class WavWriter final
{
private:
    std::ofstream file;
    size_t dataChunkPos = 0;

    template <typename Type>
    std::ostream& writeWord(std::ostream& file, Type value, size_t size = sizeof(Type))
    {
        for (; size > 0; size--, value >>= 8)
            file.put(static_cast<char>(value & 0xFF));

        return file;
    }
public:
    WavWriter(const std::string& path, uint32_t sampleRate, std::uint16_t channels);
    ~WavWriter();

    void writeSamples(const uint8_t* data, uint32_t size);
    void open(const std::string& path, uint32_t sampleRate, uint16_t channels);
    void close();
};

#endif
