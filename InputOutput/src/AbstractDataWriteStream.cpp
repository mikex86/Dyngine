#include <Stream/AbstractDataWriteStream.hpp>
#include <Stream/AbstractDataReadStream.hpp>

using namespace Stream;

EXCEPTION_TYPE_DEFAULT_IMPL(StreamOverflowException);

#define CHECK_POSITION(neededCapacity) \
if (size != -1 && (position - startPosition) + (neededCapacity) > size)  \
RAISE_EXCEPTION(StreamUnderflowException, "Tried to read from an exhausted stream")

void AbstractDataWriteStream::writeInt8(int8_t int8) {
    CHECK_POSITION(1);
    writeUint8(static_cast<uint8_t>(int8));
}

void AbstractDataWriteStream::writeUint16(uint16_t uint16) {
    CHECK_POSITION(2);
    writeUint8(uint16 >> 8);
    writeUint8(uint16);
}

void AbstractDataWriteStream::writeInt16(int16_t int16) {
    CHECK_POSITION(2);
    writeUint8(int16 >> 8);
    writeUint8(int16);
}


void AbstractDataWriteStream::writeUint32(uint32_t uint32) {
    CHECK_POSITION(4);
    writeUint8(uint32 >> 24);
    writeUint8(uint32 >> 16);
    writeUint8(uint32 >> 8);
    writeUint8(uint32);
}

void AbstractDataWriteStream::writeInt32(int32_t int32) {
    CHECK_POSITION(4);
    writeUint8(int32 >> 24);
    writeUint8(int32 >> 16);
    writeUint8(int32 >> 8);
    writeUint8(int32);
}

void AbstractDataWriteStream::writeUint64(uint64_t uint64) {
    CHECK_POSITION(8);
    writeUint8(uint64 >> 54);
    writeUint8(uint64 >> 48);
    writeUint8(uint64 >> 40);
    writeUint8(uint64 >> 32);
    writeUint8(uint64 >> 24);
    writeUint8(uint64 >> 16);
    writeUint8(uint64 >> 8);
    writeUint8(uint64);
}

void AbstractDataWriteStream::writeInt64(int64_t int64) {
    CHECK_POSITION(8);
    writeUint8(int64 >> 54);
    writeUint8(int64 >> 48);
    writeUint8(int64 >> 40);
    writeUint8(int64 >> 32);
    writeUint8(int64 >> 24);
    writeUint8(int64 >> 16);
    writeUint8(int64 >> 8);
    writeUint8(int64);
}

void AbstractDataWriteStream::writeString(const std::string &string) {
    size_t strSize = string.length();
    writeUint64(strSize);
    for (char character: string) {
        writeUint8(character);
    }
}

void AbstractDataWriteStream::writeFixedString(const std::string &string, size_t fixedLength) {
    size_t strSize = string.length();
    if (strSize > fixedLength) {
        RAISE_EXCEPTION(StreamOverflowException, "Tried to write a string with more characters than the fixed length");
    }
    for (char character: string) {
        writeUint8(character);
    }
    for (size_t i = 0; i < fixedLength - strSize; i++) {
        writeUint8(0);
    }
}

void AbstractDataWriteStream::writeBuffer(const uint8_t *buffer, size_t size) {
    CHECK_POSITION(size);
    for (size_t i = 0; i < size; i++) {
        writeUint8(buffer[i]);
    }
}

AbstractDataWriteStream::AbstractDataWriteStream(uint64_t size, uint64_t position) : size(size),
                                                                                     position(position),
                                                                                     startPosition(position) {}

uint64_t AbstractDataWriteStream::getPosition() const {
    return position;
}

uint64_t AbstractDataWriteStream::getSize() const {
    return size;
}
