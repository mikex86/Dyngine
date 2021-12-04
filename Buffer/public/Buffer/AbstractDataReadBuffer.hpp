#pragma once

#include <Buffer/DataReadBuffer.hpp>
#include <ErrorHandling/ErrorHandling.hpp>

namespace buffer {

    NEW_EXCEPTION_TYPE(BufferUnderflowException);

    class AbstractDataReadBuffer : public DataReadBuffer {

    protected:
        uint64_t size;
        uint64_t position;

        explicit AbstractDataReadBuffer(uint64_t size, uint64_t position);

    public:

        int8_t readInt8() override;

        uint16_t readUint16() override;

        int16_t readInt16() override;

        uint32_t readUint32() override;

        int32_t readInt32() override;

        uint64_t readUint64() override;

        int64_t readInt64() override;

        std::string readString() override;

        [[nodiscard]] uint64_t getSize() const;

        [[nodiscard]] uint64_t getPosition() const;

    };

}