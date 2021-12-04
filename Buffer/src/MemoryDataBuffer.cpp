#include <Buffer/MemoryDataBuffer.hpp>
#include <string>

using namespace buffer;

MemoryReadBuffer::MemoryReadBuffer(uint8_t *memory, size_t size) : AbstractDataReadBuffer(size, 0), memory(memory) {
}

MemoryReadBuffer MemoryReadBuffer::wrap(uint8_t *memory, size_t size) {
    return {memory, size};
}

#define CHECK_POSITION(neededCapacity) \
if (position + (neededCapacity) > size)  \
RAISE_EXCEPTION(BufferUnderflowException, "Tried to read from an exhausted buffer")

uint8_t MemoryReadBuffer::readUint8() {
    CHECK_POSITION(1);
    uint8_t uint8 = memory[position];
    position++;
    return uint8;
}

void MemoryReadBuffer::seek(size_t newPosition) {
    if (newPosition >= size) {
        RAISE_EXCEPTION(BufferUnderflowException,
                        "Failed to seek to position " + std::to_string(newPosition) + ", which exceeds buffer size " +
                        std::to_string(size)
        );
    }
    position = newPosition;
}

void MemoryReadBuffer::skip(size_t offset) {
    size_t newPosition = position + offset;
    if (newPosition >= size) {
        RAISE_EXCEPTION(BufferUnderflowException,
                        "Failed to skip by " + std::to_string(offset) + " to position " + std::to_string(newPosition) +
                        ", which exceeds buffer size " +
                        std::to_string(size)
        );
    }
    position = newPosition;
}

MemoryReadBuffer::~MemoryReadBuffer() = default;