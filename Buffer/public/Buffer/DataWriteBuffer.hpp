#pragma once

#include <cstdint>

namespace buffer {

    class DataWriteBuffer {

    public:

        ~DataWriteBuffer() = default;

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

        virtual void writeStreamContents(std::ifstream &stream) = 0;

        virtual void writeString(const std::string &string) = 0;
    };

}