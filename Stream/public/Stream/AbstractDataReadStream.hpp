#pragma once

#include <Stream/DataReadStream.hpp>
#include <ErrorHandling/ErrorHandling.hpp>

namespace Stream {

    NEW_EXCEPTION_TYPE(StreamUnderflowException);

    class AbstractDataReadStream : public DataReadStream {

    protected:
        uint64_t size;
        uint64_t startPosition;
        uint64_t position;

        explicit AbstractDataReadStream(uint64_t size, uint64_t position);

    public:

        int8_t readInt8() override;

        uint16_t readUint16() override;

        int16_t readInt16() override;

        uint32_t readUint32() override;

        int32_t readInt32() override;

        uint64_t readUint64() override;

        int64_t readInt64() override;

        float readFloat32() override;

        std::string readString() override;

        std::string readFixedString(size_t length) override;

        size_t read(uint8_t *buffer, size_t bufferLength) override;

        [[nodiscard]] bool hasRemaining() const override;

        [[nodiscard]] virtual uint64_t getLength() const override;

        [[nodiscard]] virtual uint64_t getPosition() const override;
    };

}