#include <gtest/gtest.h>
#include <Stream/FileDataReadStream.hpp>
#include <Stream/StreamExcept.hpp>

TEST(FileDataReadStream, Open) {
    Stream::FileDataReadStream stream = Stream::FileDataReadStream::Open("test.txt");
    EXPECT_EQ(stream.getFilePath(), "test.txt");
    ASSERT_EQ(stream.getLength(), 16);
    ASSERT_EQ(stream.getPosition(), 0);
}

TEST(FileDataReadStream, OpenNonExistant) {
    ASSERT_THROW(Stream::FileDataReadStream::Open("doesnotexist.txt"), Stream::FileDataReadStreamOpenFailedException);
}

TEST(FileDataReadStream, ReadUntilEmpty) {
    Stream::FileDataReadStream stream = Stream::FileDataReadStream::Open("test.txt");
    const size_t nCharsIncludingNullTerminator = 17;
    const char chars[nCharsIncludingNullTerminator] = "this is a test!!";
    size_t i = 0;
    while (stream.hasRemaining()) {
        ASSERT_EQ(stream.readUint8(), chars[i]);
        i++;
        if (i > nCharsIncludingNullTerminator - 1) {
            GTEST_FATAL_FAILURE_("Too many characters read");
        }
    }
}

TEST(FileDataReadStream, ReadUntilEmptyAfterSeek) {
    Stream::FileDataReadStream stream = Stream::FileDataReadStream::Open("bigger_test.txt");
    ASSERT_EQ(stream.getLength(), 533);
    ASSERT_EQ(stream.getPosition(), 0);

    stream.seek(284);
    ASSERT_EQ(stream.getPosition(), 284);

    const size_t nCharsIncludingNullTerminator = 7;
    const char chars[nCharsIncludingNullTerminator] = "Great!";
    for (size_t i = 0; i < nCharsIncludingNullTerminator - 1; i++) {
        ASSERT_EQ(stream.readUint8(), chars[i]);
    }

    stream.seek(53);
    ASSERT_EQ(stream.getPosition(), 53);

    const size_t nCharsIncludingNewLineAndNullTerminator = 57;
    const char chars2[nCharsIncludingNewLineAndNullTerminator] = "This file serves no other purpose besides exactly this.\n";
    for (size_t i = 0; i < nCharsIncludingNewLineAndNullTerminator - 1; i++) {
        ASSERT_EQ(stream.readUint8(), chars2[i]);
    }

    stream.seek(525);
    ASSERT_EQ(stream.getPosition(), 525);

    const size_t nCharsIncludingNullTerminator2 = 9;
    const char chars3[nCharsIncludingNullTerminator2] = "testing.";
    for (size_t i = 0; i < nCharsIncludingNullTerminator2 - 1; i++) {
        ASSERT_EQ(stream.readUint8(), chars3[i]);
    }
    ASSERT_TRUE(!stream.hasRemaining());

    ASSERT_THROW(stream.readUint8(), Stream::StreamUnderflowException);
}

TEST(FileDataReadStream, SeekPastEnd) {
    Stream::FileDataReadStream stream = Stream::FileDataReadStream::Open("test.txt");
    ASSERT_EQ(stream.getLength(), 16);
    ASSERT_EQ(stream.getPosition(), 0);

    ASSERT_THROW(stream.seek(17), Stream::StreamSeekException);
}

TEST(FileDataReadStream, SkipPastEnd) {
    Stream::FileDataReadStream stream = Stream::FileDataReadStream::Open("test.txt");
    ASSERT_EQ(stream.getLength(), 16);
    ASSERT_EQ(stream.getPosition(), 0);

    stream.skip(12);

    ASSERT_THROW(stream.skip(5), Stream::StreamSeekException);
}

TEST(FileDataReadStream, LargeFile) {
    Stream::FileDataReadStream stream = Stream::FileDataReadStream::Open("large_file.bin");
    size_t bufferLength = 121233416;
    ASSERT_EQ(stream.getLength(), 121233416);
    ASSERT_EQ(stream.getPosition(), 0);

    auto *buffer = new uint8_t[bufferLength];
    stream.read(buffer, bufferLength);
}