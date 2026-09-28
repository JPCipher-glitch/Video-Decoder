#ifndef SERIALIZER_HPP
#define SERIALIZER_HPP

#include <vector>
#include <string>
#include <stdexcept>

using byte = uint8_t;
using byte_stream = std::vector<uint8_t>;

class Serializer
{
protected:
	byte_stream stream;
    size_t offset = 0;
public:
    Serializer() = default;
    Serializer(const byte_stream& data) : stream{data}, offset{0} {}

    template<typename T>
    void write(const T& data)
    {
        // Check if the type is invalid
        static_assert(std::is_standard_layout_v<T> && std::is_trivial_v<T>, "Type not supported");

        // Write the data into the byte stream
        const uint8_t* dataObj = reinterpret_cast<const uint8_t*>(&data);
        stream.insert(stream.end(), dataObj, dataObj + sizeof(T));
    }

    template<typename T>
    T read()
    {
        // Check if the packet has the good size
        if (offset + sizeof(T) > stream.size())
            throw std::runtime_error("Stream out range");

        // Convert the bytes into the new obj
        T data;
        std::memcpy(&data, stream.data() + offset, sizeof(T));
        offset += sizeof(T);

        return data;
    }

    byte_stream returnStream()
    {
        byte_stream stream_copy{ stream };
        stream.clear();
        offset = 0;

        return stream_copy;
    }

    void writeString(const std::string& str) 
    {
        // Write the size into the byte stream
        uint32_t size = static_cast<uint32_t>(str.size());
        write(size);

        // Convert the string into a data
        const uint8_t* data = reinterpret_cast<const uint8_t*>(str.data());
        stream.insert(stream.end(), data, data + size);
    }

    std::string readString() 
    {
        // Check if the string is too long or corrupted
        uint32_t size = read<uint32_t>();
        if (offset + size > stream.size())
            throw std::runtime_error("ERROR: Invalid string data");

        std::string str(reinterpret_cast<const char*>(stream.data() + offset), size);
        offset += size;
        return str;
    }
};

#endif