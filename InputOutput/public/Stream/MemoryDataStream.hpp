#pragma once

#include <Stream/AbstractDataReadStream.hpp>

namespace Stream {

    class MemoryReadStream : public AbstractDataReadStream {

    private:
        const uint8_t *memory;

    public:
        MemoryReadStream(const uint8_t *memory, size_t size);

        ~MemoryReadStream() override;

        static MemoryReadStream wrap(uint8_t *memory, size_t size);

        uint8_t readUint8() override;

        void seek(size_t newPosition) override;

        void skip(size_t offset) override;

    };

}