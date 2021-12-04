#pragma once

#include <Buffer/AbstractDataReadBuffer.hpp>

namespace buffer {

    class MemoryReadBuffer : public AbstractDataReadBuffer {

    private:
        uint8_t *memory;

    public:
        MemoryReadBuffer(uint8_t *memory, size_t size);

        ~MemoryReadBuffer();

        static MemoryReadBuffer wrap(uint8_t *memory, size_t size);

        uint8_t readUint8() override;

        void seek(size_t newPosition) override;

        void skip(size_t offset) override;

    };

}