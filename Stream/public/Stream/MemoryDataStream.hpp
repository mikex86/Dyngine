#pragma once

#include <Stream/AbstractDataReadStream.hpp>
#include <vector>
#include <memory>

namespace Stream {

    class MemoryReadStream : public AbstractDataReadStream {

        const uint8_t *memory;

    public:
        /**
         *
         * @param memory the memory to read from
         * @param size the size of the memory
         * @param copyMemory if true, the memory will be copied, otherwise it will be used directly
         * @note if copyMemory is false, the memory must be valid for the lifetime of the stream.
         * You are responsible for freeing the memory.
         */
        MemoryReadStream(const uint8_t *memory, size_t size, bool copyMemory);

        ~MemoryReadStream() override;

        /**
         * @param memory the memory to read from.
         * The stream will store a copy of the given memory and free it when the stream is destroyed.
         * @param size the size of the memory
         */
        static std::unique_ptr<MemoryReadStream> CopyOf(const uint8_t *memory, size_t size);

        /**
         * @param memory the memory to read from. The stream will use the memory directly.
         * The memory must be valid for the lifetime of the stream.
         * You are responsible for freeing the memory.
         * @param size the size of the memory
         */
        static std::unique_ptr<MemoryReadStream> Wrap(const uint8_t *memory, size_t size);

        uint8_t readUint8() override;

        void seek(uint64_t newPosition) override;

        void skip(uint64_t offset) override;

    };

}