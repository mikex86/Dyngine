#include <Buffer/AbstractDataReadBuffer.hpp>
#include <vector>

using namespace buffer;

#define CHECK_POSITION(neededCapacity) \
if (position + (neededCapacity) > size)  \
RAISE_EXCEPTION(BufferUnderflowException, "Tried to read from an exhausted buffer")

EXCEPTION_TYPE_DEFAULT_IMPL(BufferUnderflowException);

int8_t AbstractDataReadBuffer::readInt8() {
    return static_cast<int8_t>(readUint8());
}

uint16_t AbstractDataReadBuffer::readUint16() {
    CHECK_POSITION(2);
    auto b1 = readUint8();
    auto b2 = readUint8();
    return static_cast<uint16_t>(b1 << 8 | b2);
}

int16_t AbstractDataReadBuffer::readInt16() {
    CHECK_POSITION(2);
    auto b1 = readUint8();
    auto b2 = readUint8();
    return static_cast<int16_t>(b1 << 8 | b2);
}

uint32_t AbstractDataReadBuffer::readUint32() {
    CHECK_POSITION(4);
    auto b1 = readUint8();
    auto b2 = readUint8();
    auto b3 = readUint8();
    auto b4 = readUint8();
    return static_cast<uint32_t>(b1 << 24 | b2 << 16 | b3 << 8 | b4);
}

int32_t AbstractDataReadBuffer::readInt32() {
    CHECK_POSITION(4);
    auto b1 = readUint8();
    auto b2 = readUint8();
    auto b3 = readUint8();
    auto b4 = readUint8();
    return static_cast<int32_t>(b1 << 24 | b2 << 16 | b3 << 8 | b4);
}

uint64_t AbstractDataReadBuffer::readUint64() {
    CHECK_POSITION(8);
    auto b1 = static_cast<uint64_t>(readUint8());
    auto b2 = static_cast<uint64_t>(readUint8());
    auto b3 = static_cast<uint64_t>(readUint8());
    auto b4 = static_cast<uint64_t>(readUint8());
    auto b5 = static_cast<uint64_t>(readUint8());
    auto b6 = static_cast<uint64_t>(readUint8());
    auto b7 = static_cast<uint64_t>(readUint8());
    auto b8 = static_cast<uint64_t>(readUint8());
    return b1 << 56 | b2 << 48 | b3 << 40 | b4 << 32 | b5 << 24 | b6 << 16 | b7 << 8 | b8;
}

int64_t AbstractDataReadBuffer::readInt64() {
    CHECK_POSITION(8);
    auto b1 = static_cast<uint64_t>(readUint8());
    auto b2 = static_cast<uint64_t>(readUint8());
    auto b3 = static_cast<uint64_t>(readUint8());
    auto b4 = static_cast<uint64_t>(readUint8());
    auto b5 = static_cast<uint64_t>(readUint8());
    auto b6 = static_cast<uint64_t>(readUint8());
    auto b7 = static_cast<uint64_t>(readUint8());
    auto b8 = static_cast<uint64_t>(readUint8());
    return static_cast<int64_t>(b1 << 56 | b2 << 48 | b3 << 40 | b4 << 32 | b5 << 24 | b6 << 16 | b7 << 8 | b8);
}


std::string AbstractDataReadBuffer::readString() {
    uint64_t length = readInt64();
    std::vector<char> buffer;
    buffer.reserve(length);
    for (uint64_t i = 0; i < length; i++) {
        char character = static_cast<char>(readUint8());
        buffer.push_back(character);
    }
    buffer.push_back('\0');
    return {buffer.data()};
}

AbstractDataReadBuffer::AbstractDataReadBuffer(uint64_t size, uint64_t position) : size(size), position(position) {
}

size_t AbstractDataReadBuffer::getSize() const {
    return size;
}

size_t AbstractDataReadBuffer::getPosition() const {
    return position;
}