#include <Buffer/AbstractDataWriteBuffer.hpp>

using namespace buffer;

EXCEPTION_TYPE_DEFAULT_IMPL(BufferOverflowException);

#define CHECK_POSITION(neededCapacity) \
if (size != -1 && position + (neededCapacity) > size)  \
RAISE_EXCEPTION(BufferOverflowException, "Tried to write to an exhausted buffer")

void AbstractDataWriteBuffer::writeInt8(int8_t int8) {
    CHECK_POSITION(1);
    writeUint8(static_cast<uint8_t>(int8));
}

void AbstractDataWriteBuffer::writeUint16(uint16_t uint16) {
    CHECK_POSITION(2);
    writeUint8(uint16 >> 8);
    writeUint8(uint16);
}

void AbstractDataWriteBuffer::writeInt16(int16_t int16) {
    CHECK_POSITION(2);
    writeUint8(int16 >> 8);
    writeUint8(int16);
}


void AbstractDataWriteBuffer::writeUint32(uint32_t uint32) {
    CHECK_POSITION(4);
    writeUint8(uint32 >> 24);
    writeUint8(uint32 >> 16);
    writeUint8(uint32 >> 8);
    writeUint8(uint32);
}

void AbstractDataWriteBuffer::writeInt32(int32_t int32) {
    CHECK_POSITION(4);
    writeUint8(int32 >> 24);
    writeUint8(int32 >> 16);
    writeUint8(int32 >> 8);
    writeUint8(int32);
}

void AbstractDataWriteBuffer::writeUint64(uint64_t uint64) {
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

void AbstractDataWriteBuffer::writeInt64(int64_t int64) {
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

void AbstractDataWriteBuffer::writeString(const std::string &string) {
    size_t strSize = string.length();
    writeUint64(strSize);
    for (char character: string) {
        writeUint8(character);
    }
}

AbstractDataWriteBuffer::AbstractDataWriteBuffer(uint64_t size, uint64_t position) : size(size),
                                                                                     position(position) {}

uint64_t AbstractDataWriteBuffer::getPosition() const {
    return position;
}

uint64_t AbstractDataWriteBuffer::getSize() const {
    return size;
}

