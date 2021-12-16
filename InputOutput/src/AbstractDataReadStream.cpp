#include <Stream/AbstractDataReadStream.hpp>
#include <vector>

using namespace Stream;

#define CHECK_POSITION(neededCapacity) \
if (size != -1 && (position - startPosition) + (neededCapacity) > size)  \
RAISE_EXCEPTION(StreamUnderflowException, "Tried to read from an exhausted stream")

EXCEPTION_TYPE_DEFAULT_IMPL(StreamUnderflowException);

int8_t AbstractDataReadStream::readInt8() {
    return static_cast<int8_t>(readUint8());
}

uint16_t AbstractDataReadStream::readUint16() {
    CHECK_POSITION(2);
    auto b1 = readUint8();
    auto b2 = readUint8();
    return static_cast<uint16_t>(b1 << 8 | b2);
}

int16_t AbstractDataReadStream::readInt16() {
    CHECK_POSITION(2);
    auto b1 = readUint8();
    auto b2 = readUint8();
    return static_cast<int16_t>(b1 << 8 | b2);
}

uint32_t AbstractDataReadStream::readUint32() {
    CHECK_POSITION(4);
    auto b1 = readUint8();
    auto b2 = readUint8();
    auto b3 = readUint8();
    auto b4 = readUint8();
    return static_cast<uint32_t>(b1 << 24 | b2 << 16 | b3 << 8 | b4);
}

int32_t AbstractDataReadStream::readInt32() {
    CHECK_POSITION(4);
    auto b1 = readUint8();
    auto b2 = readUint8();
    auto b3 = readUint8();
    auto b4 = readUint8();
    return static_cast<int32_t>(b1 << 24 | b2 << 16 | b3 << 8 | b4);
}

uint64_t AbstractDataReadStream::readUint64() {
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

int64_t AbstractDataReadStream::readInt64() {
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

size_t AbstractDataReadStream::read(uint8_t *buffer, size_t bufferLength) {
    for (size_t i = 0; i < bufferLength; i++) {
        if (size != - 1 && position - startPosition >= size) {
            return i;
        }
        buffer[i] = readUint8();
    }
    return bufferLength;
}


std::string AbstractDataReadStream::readString() {
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


std::string AbstractDataReadStream::readFixedString(size_t length) {
    std::string str;
    str.resize(length);
    read(reinterpret_cast<uint8_t *>(&str[0]), length);
    str = str.substr(0, str.find_first_of('\0'));
    return str;
}

AbstractDataReadStream::AbstractDataReadStream(uint64_t size, uint64_t position) : size(size), position(position), startPosition(position) {
}

uint64_t AbstractDataReadStream::getSize() const {
    return size;
}

uint64_t AbstractDataReadStream::getPosition() const {
    return position;
}

bool AbstractDataReadStream::hasRemaining() const {
    return size == -1 || (position - startPosition) < size;
}
