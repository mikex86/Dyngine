#pragma once

#include <cstdint>
#include <string>

namespace buffer {

    class DataReadBuffer {

    public:

        ~DataReadBuffer() = default;

        // Read functions
        virtual uint8_t readUint8() = 0;

        virtual int8_t readInt8() = 0;

        virtual uint16_t readUint16() = 0;

        virtual int16_t readInt16() = 0;

        virtual uint32_t readUint32() = 0;

        virtual int32_t readInt32() = 0;

        virtual uint64_t readUint64() = 0;

        virtual int64_t readInt64() = 0;

        virtual std::string readString() = 0;

        // Seek functions
        virtual void seek(uint64_t position) = 0;

        virtual void skip(uint64_t offset) = 0;

    };

}