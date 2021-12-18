#include <gtest/gtest.h>
#include <Stream/MemoryDataStream.hpp>

TEST(MemoryReadStream, Wrap) {
    const size_t size = 10;
    auto *memory = new uint8_t[size];
    Stream::MemoryReadStream stream = Stream::MemoryReadStream::CopyOf(memory, size);
    ASSERT_EQ(0, stream.getPosition());
    ASSERT_EQ(size, stream.getLength());
}

TEST(MemoryReadStream, ReadUint8) {
    const size_t size = 10;
    auto *memory = new uint8_t[size]{10, 0, 0, 43};
    Stream::MemoryReadStream stream = Stream::MemoryReadStream::CopyOf(memory, size);

    ASSERT_EQ(0, stream.getPosition());
    ASSERT_EQ(size, stream.getLength());

    EXPECT_EQ(10, stream.readUint8());

    ASSERT_TRUE(stream.hasRemaining());

    ASSERT_EQ(1, stream.getPosition());
}


TEST(MemoryReadStream, Seek) {
    const size_t size = 4;
    auto *memory = new uint8_t[size]{10, 0, 0, 43};
    Stream::MemoryReadStream stream = Stream::MemoryReadStream::CopyOf(memory, size);

    ASSERT_EQ(0, stream.getPosition());
    ASSERT_EQ(size, stream.getLength());

    EXPECT_EQ(10, stream.readUint8());

    ASSERT_TRUE(stream.hasRemaining());
    ASSERT_EQ(1, stream.getPosition());

    stream.seek(3);
    EXPECT_EQ(43, stream.readUint8());
    ASSERT_FALSE(stream.hasRemaining());
}

TEST(MemoryReadStream, SeekPastEnd) {
    const size_t size = 4;
    auto *memory = new uint8_t[size]{10, 0, 0, 43};
    Stream::MemoryReadStream stream = Stream::MemoryReadStream::CopyOf(memory, size);

    ASSERT_EQ(0, stream.getPosition());
    ASSERT_EQ(size, stream.getLength());

    EXPECT_EQ(10, stream.readUint8());

    ASSERT_TRUE(stream.hasRemaining());
    ASSERT_EQ(1, stream.getPosition());

    ASSERT_THROW(stream.seek(4), Stream::StreamUnderflowException);
}

TEST(MemoryReadStream, Skip) {
    const size_t size = 4;
    auto *memory = new uint8_t[size]{10, 0, 0, 43};
    Stream::MemoryReadStream stream = Stream::MemoryReadStream::CopyOf(memory, size);

    ASSERT_EQ(0, stream.getPosition());
    ASSERT_EQ(size, stream.getLength());

    EXPECT_EQ(10, stream.readUint8());

    ASSERT_TRUE(stream.hasRemaining());
    ASSERT_EQ(1, stream.getPosition());

    stream.skip(2);
    EXPECT_EQ(43, stream.readUint8());
    ASSERT_FALSE(stream.hasRemaining());
}

TEST(MemoryReadStream, SkipPastEnd) {
    const size_t size = 4;
    auto *memory = new uint8_t[size]{10, 0, 0, 43};
    Stream::MemoryReadStream stream = Stream::MemoryReadStream::CopyOf(memory, size);

    ASSERT_EQ(0, stream.getPosition());
    ASSERT_EQ(size, stream.getLength());

    EXPECT_EQ(10, stream.readUint8());

    ASSERT_TRUE(stream.hasRemaining());
    ASSERT_EQ(1, stream.getPosition());

    ASSERT_THROW(stream.skip(3), Stream::StreamUnderflowException);
}