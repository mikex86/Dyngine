#include <gtest/gtest.h>
#include <Stream/FileDataWriteStream.hpp>
#include <Stream/FileDataReadStream.hpp>
#include <Stream/ZstdDeflateStream.hpp>
#include <Stream/ZstdInflateStream.hpp>

TEST(ZstdStreamTest, DeflateInfateTest) {
    // compress
    {
        auto inputStream = Stream::FileDataReadStream::Open("large_file.bin");
        auto outputStream = Stream::FileDataWriteStream::Open("compressed_file.bin.zstd");
        Stream::ZstdDeflateStream deflateStream(std::shared_ptr<Stream::AbstractDataWriteStream>(&outputStream,[](
                Stream::FileDataWriteStream *) {}));
        deflateStream.writeStreamContents(inputStream);
        outputStream.close();
    }

    // decompress
    {
        auto inputStream = Stream::FileDataReadStream::Open("large_file.bin");

        auto compressedInputStream = Stream::FileDataReadStream::Open("compressed_file.bin.zstd");
        Stream::ZstdInflateStream
        inflateStream(std::shared_ptr<Stream::AbstractDataReadStream>(&compressedInputStream,[](Stream::FileDataReadStream *) {}));

        while (inputStream.hasRemaining()) {
            ASSERT_EQ(inputStream.readUint8(), inflateStream.readUint8());
        }
    }
}