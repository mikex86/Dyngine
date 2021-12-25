#include <Stream/MemoryDataStream.hpp>
#include <string>

using namespace Stream;

MemoryReadStream::MemoryReadStream(const uint8_t *memory, size_t size, bool copyMemory)
        : AbstractDataReadStream(size, 0) {
    if (copyMemory) {
        this->memory = new uint8_t[size];
        memcpy(const_cast<uint8_t *>(this->memory), memory, size);
    } else {
        this->memory = memory;
    }
}

std::unique_ptr<MemoryReadStream> MemoryReadStream::CopyOf(const uint8_t *memory, size_t size) {
    return std::make_unique<MemoryReadStream>(memory, size, true);
}

std::unique_ptr<MemoryReadStream> MemoryReadStream::Wrap(const uint8_t *memory, size_t size) {
    return std::make_unique<MemoryReadStream>(memory, size, false);
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

void MemoryReadStream::seek(uint64_t newPosition) {
    if (newPosition >= size) {
        RAISE_EXCEPTION(StreamUnderflowException,
                        "Failed to seek to position " + std::to_string(newPosition) + ", which exceeds buffer size " +
                        std::to_string(size)
        );
    }
    position = newPosition;
}

void MemoryReadStream::skip(uint64_t offset) {
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

MemoryReadStream::~MemoryReadStream() {
    delete[] memory;
}