#include <gtest/gtest.h>
#include <Dpac/Dpac.hpp>
#include <Stream/MemoryDataStream.hpp>

TEST(DpacArchive, DpacWriteAndReadTest) {
    const size_t memoryContentLength1 = 64;
    const char memoryContent1[memoryContentLength1] = "This is some random ass content which is stored in this archive";

    const size_t memoryContentLength2 = 73;
    const char memoryContent2[memoryContentLength2] = "Yeet, yet some more random file content, which is stored in this archive";

    const size_t memoryContentLength3 = 93;
    const char memoryContent3[memoryContentLength3] = "This is some more content which is stored in this archive. This is getting boring, isn't it?";

    // write archive
    {
        Dpac::WriteOnlyArchive writeArchive = Dpac::WriteOnlyArchive::Open("DpacArchiveTest.dpac");
        writeArchive.reserveNEntries(3);
        writeArchive.finalizeEntryTable();
        {
            auto memoryStream = Stream::MemoryReadStream(reinterpret_cast<const uint8_t *>(memoryContent1),
                                                         memoryContentLength1);
            writeArchive.defineEntryFromUncompressedStream(0, "/entry1.txt", memoryStream);
        }
        {
            auto memoryStream = Stream::MemoryReadStream(reinterpret_cast<const uint8_t *>(memoryContent2),
                                                         memoryContentLength2);
            writeArchive.defineEntryFromUncompressedStream(1, "/entry2.txt", memoryStream);
        }
        {
            auto memoryStream = Stream::MemoryReadStream(reinterpret_cast<const uint8_t *>(memoryContent3),
                                                         memoryContentLength3);
            writeArchive.defineEntryFromUncompressedStream(2, "/entry3.txt", memoryStream);
        }
    }

    // read archive
    {
        Dpac::ReadOnlyArchive readArchive = Dpac::ReadOnlyArchive::Open("DpacArchiveTest.dpac");
        {
            auto streamSize = readArchive.getUncompressedEntrySize("/entry1.txt");
            auto stream = readArchive.getEntryStream("/entry1.txt");
            auto *buffer = new uint8_t[streamSize];
            stream->read(buffer, streamSize);
            EXPECT_EQ(0, memcmp(buffer, memoryContent1, streamSize));
        }
        {
            auto streamSize = readArchive.getUncompressedEntrySize("/entry2.txt");
            auto stream = readArchive.getEntryStream("/entry2.txt");
            auto *buffer = new uint8_t[streamSize];
            stream->read(buffer, streamSize);
            EXPECT_EQ(0, memcmp(buffer, memoryContent2, streamSize));
        }
        {
            auto streamSize = readArchive.getUncompressedEntrySize("/entry3.txt");
            auto stream = readArchive.getEntryStream("/entry3.txt");
            auto *buffer = new uint8_t[streamSize];
            stream->read(buffer, streamSize);
            EXPECT_EQ(0, memcmp(buffer, memoryContent3, streamSize));
        }
    }
}