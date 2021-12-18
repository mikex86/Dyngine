#include <gtest/gtest.h>
#include <Stream/AbstractDataReadStream.hpp>
#include <Stream/MemoryDataStream.hpp>


TEST(AbstractDataReadStream, ReadUint8) {
    const size_t bufferSize = 10;
    auto *memory = new uint8_t[bufferSize]{245, 32, 0, 255, 52, 12, 40, 54, 198, 13};
    Stream::MemoryReadStream stream(memory, bufferSize, false);
    ASSERT_EQ(0, stream.getPosition());
    ASSERT_EQ(bufferSize, stream.getLength());

    EXPECT_EQ(245, stream.readUint8());
    EXPECT_EQ(32, stream.readUint8());
    EXPECT_EQ(0, stream.readUint8());
    EXPECT_EQ(255, stream.readUint8());
    EXPECT_EQ(52, stream.readUint8());
    EXPECT_EQ(12, stream.readUint8());
    EXPECT_EQ(40, stream.readUint8());
    EXPECT_EQ(54, stream.readUint8());
    EXPECT_EQ(198, stream.readUint8());
    EXPECT_EQ(13, stream.readUint8());
    EXPECT_EQ(bufferSize, stream.getPosition());

    ASSERT_TRUE(!stream.hasRemaining());
}

TEST(AbstractDataReadStream, ReadInt8) {
    const size_t bufferSize = 10;
    auto *memory = new uint8_t[bufferSize]{245, 111, 0, 169, 20, 183, 129, 178, 203, 103};
    Stream::MemoryReadStream stream(memory, bufferSize, false);
    ASSERT_EQ(0, stream.getPosition());
    ASSERT_EQ(bufferSize, stream.getLength());

    EXPECT_EQ(-11, stream.readInt8());
    EXPECT_EQ(111, stream.readInt8());
    EXPECT_EQ(0, stream.readInt8());
    EXPECT_EQ(-87, stream.readInt8());
    EXPECT_EQ(20, stream.readInt8());
    EXPECT_EQ(-73, stream.readInt8());
    EXPECT_EQ(-127, stream.readInt8());
    EXPECT_EQ(-78, stream.readInt8());
    EXPECT_EQ(-53, stream.readInt8());
    EXPECT_EQ(103, stream.readInt8());
    EXPECT_EQ(bufferSize, stream.getPosition());

    ASSERT_TRUE(!stream.hasRemaining());
}

TEST(AbstractDataReadStream, ReadUint16) {
    const size_t bufferSize = 20;
    auto *memory = new uint8_t[bufferSize]{
            9, 17, 0, 125, 125, 23, 250, 123, 28, 142, 24, 86, 17, 230, 38, 165, 50, 123, 245, 203
    };
    Stream::MemoryReadStream stream(memory, bufferSize, false);
    ASSERT_EQ(0, stream.getPosition());
    ASSERT_EQ(bufferSize, stream.getLength());

    EXPECT_EQ(2321, stream.readUint16());
    EXPECT_EQ(125, stream.readUint16());
    EXPECT_EQ(32023, stream.readUint16());
    EXPECT_EQ(64123, stream.readUint16());
    EXPECT_EQ(7310, stream.readUint16());
    EXPECT_EQ(6230, stream.readUint16());
    EXPECT_EQ(4582, stream.readUint16());
    EXPECT_EQ(9893, stream.readUint16());
    EXPECT_EQ(12923, stream.readUint16());
    EXPECT_EQ(62923, stream.readUint16());
    EXPECT_EQ(bufferSize, stream.getPosition());

    ASSERT_TRUE(!stream.hasRemaining());
}

TEST(AbstractDataReadStream, ReadInt16) {
    const size_t bufferSize = 20;
    auto *memory = new uint8_t[bufferSize]{246, 176, 17, 230, 134, 87, 125, 0, 219, 124, 250, 142, 250, 47, 1, 138, 255,
                                           233, 255, 255,};
    Stream::MemoryReadStream stream(memory, bufferSize, false);
    ASSERT_EQ(0, stream.getPosition());
    ASSERT_EQ(bufferSize, stream.getLength());

    EXPECT_EQ(-2384, stream.readInt16());
    EXPECT_EQ(4582, stream.readInt16());
    EXPECT_EQ(-31145, stream.readInt16());
    EXPECT_EQ(32000, stream.readInt16());
    EXPECT_EQ(-9348, stream.readInt16());
    EXPECT_EQ(-1394, stream.readInt16());
    EXPECT_EQ(-1489, stream.readInt16());
    EXPECT_EQ(394, stream.readInt16());
    EXPECT_EQ(-23, stream.readInt16());
    EXPECT_EQ(-1, stream.readInt16());
    EXPECT_EQ(bufferSize, stream.getPosition());

    ASSERT_TRUE(!stream.hasRemaining());
}

TEST(AbstractDataReadStream, ReadUint32) {
    const size_t bufferSize = 40;
    auto *memory = new uint8_t[bufferSize]{
            0, 0, 3, 60, 0, 20, 182, 141, 238, 107, 40, 0, 11, 235, 194, 0, 0, 0, 28, 221, 0, 15, 57, 157, 0, 0, 178,
            114, 0, 6, 86, 236, 0, 12, 113, 108, 0, 66, 98, 103
    };
    Stream::MemoryReadStream stream(memory, bufferSize, false);
    ASSERT_EQ(0, stream.getPosition());
    ASSERT_EQ(bufferSize, stream.getLength());

    EXPECT_EQ(828, stream.readUint32());
    EXPECT_EQ(1357453, stream.readUint32());
    EXPECT_EQ(4000000000L, stream.readUint32());
    EXPECT_EQ(200000000, stream.readUint32());
    EXPECT_EQ(7389, stream.readUint32());
    EXPECT_EQ(997789, stream.readUint32());
    EXPECT_EQ(45682, stream.readUint32());
    EXPECT_EQ(415468, stream.readUint32());
    EXPECT_EQ(815468, stream.readUint32());
    EXPECT_EQ(4350567, stream.readUint32());
    EXPECT_EQ(bufferSize, stream.getPosition());

    ASSERT_TRUE(!stream.hasRemaining());
}

TEST(AbstractDataReadStream, ReadInt32) {
    const size_t bufferSize = 40;
    auto *memory = new uint8_t[bufferSize]{
            0, 0, 1, 93, 0, 4, 245, 106, 0, 0, 3, 168, 0, 0, 0, 48, 255, 255, 254, 167, 255, 255, 164, 63, 0, 0, 5, 209,
            0, 0, 1, 233, 255, 255, 77, 152, 0, 0, 10, 12
    };
    Stream::MemoryReadStream stream(memory, bufferSize, false);
    ASSERT_EQ(0, stream.getPosition());
    ASSERT_EQ(bufferSize, stream.getLength());

    EXPECT_EQ(349, stream.readInt32());
    EXPECT_EQ(324970, stream.readInt32());
    EXPECT_EQ(936, stream.readInt32());
    EXPECT_EQ(48, stream.readInt32());
    EXPECT_EQ(-345, stream.readInt32());
    EXPECT_EQ(-23489, stream.readInt32());
    EXPECT_EQ(1489, stream.readInt32());
    EXPECT_EQ(489, stream.readInt32());
    EXPECT_EQ(-45672, stream.readInt32());
    EXPECT_EQ(2572, stream.readInt32());
    EXPECT_EQ(bufferSize, stream.getPosition());

    ASSERT_TRUE(!stream.hasRemaining());
}

TEST(AbstractDataReadStream, ReadUint64) {
    const size_t bufferSize = 80;
    auto *memory = new uint8_t[bufferSize]{
            0, 0, 0, 0, 0, 0, 193, 112, 0, 0, 2, 34, 40, 2, 181, 188, 0, 0, 20, 82, 196, 231, 245, 188, 255, 255, 255,
            255, 255, 255, 255, 255, 127, 255, 255, 255, 255, 255, 255, 255, 0, 8, 85, 109, 172, 132, 253, 90, 0, 0, 0,
            0, 0, 0, 0, 234, 0, 0, 0, 0, 0, 0, 47, 202, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0
    };
    Stream::MemoryReadStream stream(memory, bufferSize, false);
    ASSERT_EQ(0, stream.getPosition());
    ASSERT_EQ(bufferSize, stream.getLength());

    EXPECT_EQ(49520, stream.readUint64());
    EXPECT_EQ(2345723409852L, stream.readUint64());
    EXPECT_EQ(22345723409852L, stream.readUint64());
    EXPECT_EQ(18446744073709551615UL, stream.readUint64());
    EXPECT_EQ(9223372036854775807L, stream.readUint64());
    EXPECT_EQ(2345729347878234L, stream.readUint64());
    EXPECT_EQ(234L, stream.readUint64());
    EXPECT_EQ(12234L, stream.readUint64());
    EXPECT_EQ(1L, stream.readUint64());
    EXPECT_EQ(0L, stream.readUint64());
    EXPECT_EQ(bufferSize, stream.getPosition());

    ASSERT_TRUE(!stream.hasRemaining());
}

TEST(AbstractDataReadStream, ReadInt64) {
    const size_t bufferSize = 80;
    auto *memory = new uint8_t[bufferSize]{
            0, 0, 0, 0, 0, 0, 0, 10, 255, 255, 255, 255, 255, 255, 255, 246, 255, 255, 255, 255, 255, 254, 26, 26, 0, 0,
            0, 0, 13, 250, 156, 239, 0, 0, 0, 0, 133, 48, 48, 239, 0, 0, 0, 23, 26, 214, 186, 239, 255, 255, 255, 255,
            252, 139, 24, 233, 255, 255, 255, 134, 30, 224, 238, 242, 0, 0, 0, 0, 4, 12, 223, 14, 0, 0, 0, 0, 0, 0, 0,
            67
    };
    Stream::MemoryReadStream stream(memory, bufferSize, false);
    ASSERT_EQ(0, stream.getPosition());
    ASSERT_EQ(bufferSize, stream.getLength());

    EXPECT_EQ(10, stream.readInt64());
    EXPECT_EQ(-10L, stream.readInt64());
    EXPECT_EQ(-124390L, stream.readInt64());
    EXPECT_EQ(234527983L, stream.readInt64());
    EXPECT_EQ(2234527983L, stream.readInt64());
    EXPECT_EQ(99234527983L, stream.readInt64());
    EXPECT_EQ(-57992983L, stream.readInt64());
    EXPECT_EQ(-523467952398L, stream.readInt64());
    EXPECT_EQ(67952398L, stream.readInt64());
    EXPECT_EQ(67L, stream.readInt64());
    EXPECT_EQ(bufferSize, stream.getPosition());

    ASSERT_TRUE(!stream.hasRemaining());
}

TEST(AbstractDataReadStream, ReadString) {
    const size_t bufferSize = 23;
    auto *memory = new uint8_t[bufferSize]{
            0, 0, 0, 0, 0, 0, 0, 15, // Length
            'T', 'h', 'i', 's', ' ', 'i', 's', ' ', 'a', ' ', 't', 'e', 's', 't', '.'
    };
    Stream::MemoryReadStream stream(memory, bufferSize, false);
    ASSERT_EQ(0, stream.getPosition());
    ASSERT_EQ(bufferSize, stream.getLength());

    ASSERT_EQ("This is a test.", stream.readString());

    ASSERT_TRUE(!stream.hasRemaining());
}


TEST(AbstractDataReadStream, ReadFixedString) {
    const size_t bufferSize = 64;
    const uint8_t memory[bufferSize] = "This is a fixed string test.";
    Stream::MemoryReadStream stream(memory, bufferSize, false);
    ASSERT_EQ(0, stream.getPosition());
    ASSERT_EQ(bufferSize, stream.getLength());

    ASSERT_EQ("This is a fixed string test.", stream.readFixedString(bufferSize));

    ASSERT_TRUE(!stream.hasRemaining());
}


TEST(AbstractDataReadStream, Read) {
    const size_t bufferSize = 16;
    const uint8_t memory[bufferSize] = {
            1, 2, 4, 114, 45, 234, 112, 234, 56, 4, 32,
            56, 3, 178, 34, 123
    };
    Stream::MemoryReadStream stream(memory, bufferSize, false);
    ASSERT_EQ(0, stream.getPosition());
    ASSERT_EQ(bufferSize, stream.getLength());

    uint8_t buffer[bufferSize];
    ASSERT_EQ(bufferSize, stream.read(buffer, bufferSize));
    ASSERT_EQ(bufferSize, stream.getPosition());
    ASSERT_EQ(0, memcmp(buffer, memory, bufferSize));

    ASSERT_TRUE(!stream.hasRemaining());
}

TEST(AbstractDataReadStream, ReadStreamUnderflow) {
    const size_t bufferSize = 16;
    const uint8_t memory[bufferSize] = {
            1, 2, 4, 114, 45, 234, 112, 234, 56, 4, 32,
            56, 3, 178, 34, 123
    };
    Stream::MemoryReadStream stream(memory, bufferSize, false);
    ASSERT_EQ(0, stream.getPosition());
    ASSERT_EQ(bufferSize, stream.getLength());

    stream.seek(5);
    ASSERT_EQ(5, stream.getPosition());

    uint8_t buffer[bufferSize];
    ASSERT_EQ(11, stream.read(buffer, bufferSize));
    ASSERT_EQ(bufferSize, stream.getPosition());
    ASSERT_EQ(0, memcmp(buffer, memory + 5, 11));

    ASSERT_TRUE(!stream.hasRemaining());
}