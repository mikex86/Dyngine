#include <Stream/MemoryDataStream.hpp>
#include <string>

using namespace Stream;

MemoryReadStream::MemoryReadStream(const uint8_t *memory, size_t size) : AbstractDataReadStream(size, 0), memory(memory) {
}

MemoryReadStream MemoryReadStream::wrap(uint8_t *memory, size_t size) {
    return {memory, size};
}

#define CHECK_POSITION(neededCapacity) \
if (position + (neededCapacity) > size)  \
RAISE_EXCEPTION(StreamUnderflowException, "Tried to read from an exhausted buffer")

uint8_t MemoryReadStream::readUint8() {
    CHECK_POSITION(1);
    uint8_t uint8 = memory[position];
    position++;
    return uint8;
}

void MemoryReadStream::seek(size_t newPosition) {
    if (newPosition >= size) {
        RAISE_EXCEPTION(StreamUnderflowException,
                        "Failed to seek to position " + std::to_string(newPosition) + ", which exceeds buffer size " +
                        std::to_string(size)
        );
    }
    position = newPosition;
}

void MemoryReadStream::skip(size_t offset) {
    size_t newPosition = position + offset;
    if (newPosition >= size) {
        RAISE_EXCEPTION(StreamUnderflowException,
                        "Failed to skip by " + std::to_string(offset) + " to position " + std::to_string(newPosition) +
                        ", which exceeds buffer size " +
                        std::to_string(size)
        );
    }
    position = newPosition;
}

MemoryReadStream::~MemoryReadStream() = default;