#pragma once

#include <ErrorHandling/ErrorHandling.hpp>
#include <Stream/DataWriteStream.hpp>

namespace Stream {

    NEW_EXCEPTION_TYPE(StreamOverflowException);

    class AbstractDataWriteStream : public DataWriteStream {

    protected:
        uint64_t size;
        uint64_t position;
        uint64_t startPosition;

        AbstractDataWriteStream(uint64_t size, uint64_t position);

    public:

        void writeInt8(int8_t int8) override;

        void writeUint16(uint16_t uint16) override;

        void writeInt16(int16_t int16) override;

        void writeUint32(uint32_t uint32) override;

        void writeInt32(int32_t int32) override;

        void writeUint64(uint64_t uint64) override;

        void writeInt64(int64_t int64) override;

        void writeString(const std::string &string) override;

        void writeFixedString(const std::string &string, size_t fixedLength) override;

        void writeBuffer(const uint8_t *buffer, size_t size) override;

        [[nodiscard]] uint64_t getPosition() const;

        [[nodiscard]] uint64_t getSize() const;
    };

}