#pragma once

#include <cstdint>
#include <Stream/DataReadStream.hpp>

namespace Stream {

    class DataWriteStream {

    public:

        ~DataWriteStream() = default;

        virtual void seek(uint64_t position) = 0;

        virtual void skip(uint64_t offset) = 0;

        virtual void writeUint8(uint8_t uint8) = 0;

        virtual void writeInt8(int8_t int8) = 0;

        virtual void writeUint16(uint16_t uint16) = 0;

        virtual void writeInt16(int16_t int16) = 0;

        virtual void writeUint32(uint32_t uint32) = 0;

        virtual void writeInt32(int32_t int32) = 0;

        virtual void writeUint64(uint64_t uint64) = 0;

        virtual void writeInt64(int64_t int64) = 0;

        /**
         * @brief Writes the contents of the supplied stream to this stream.
         * @param stream the input stream to read from
         * @return a pair of {the number of bytes written, the number of bytes read}.
         * These two values can differ in some stream implementations, eg when compression is utilized.
         */
        virtual std::pair<size_t, size_t> writeStreamContents(Stream::DataReadStream &stream) = 0;

        virtual void writeBuffer(const uint8_t *buffer, size_t size) = 0;

        virtual void writeString(const std::string &string) = 0;

        virtual void writeFixedString(const std::string &string, size_t fixedLength) = 0;

    };

}