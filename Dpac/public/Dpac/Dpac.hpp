#pragma once

#include <ErrorHandling/ErrorHandling.hpp>
#include <Stream/FileDataReadStream.hpp>
#include <Stream/FileDataWriteStream.hpp>
#include <map>
#include <vector>
#include <string>

#define DPAC_MAX_PATH 128


namespace Dpac {

    NEW_EXCEPTION_TYPE(ArchiveOpenFailedException);

    NEW_EXCEPTION_TYPE(ArchiveCloseFailedException);

    NEW_EXCEPTION_TYPE(ArchiveTooFewEntriesReservedException);

    NEW_EXCEPTION_TYPE(ArchiveEntryNotDefinedException);

    NEW_EXCEPTION_TYPE(ArchiveEntryTableNotYetFinalizedException);

    NEW_EXCEPTION_TYPE(EntryDoesNotExistException);

    NEW_EXCEPTION_TYPE(ArchiveEntryTableAlreadyFinalizedException);

    class ReadOnlyArchive {
    private:
        Stream::FileDataReadStream dataStream;

        uint64_t heapStart{};

        /**
         * Stores the offsets of different entries where they are stored in the heap.
         * The offset is relative from the heap start.
         */
        std::map<std::string, uint64_t> entryContentOffsetTable{};

        /**
         * Stores the uncompressed sizes of all entries
         */
        std::map<std::string, uint64_t> entryContentCompressedSizeTable{};

        /**
         * Stores the uncompressed sizes of all entries
         */
        std::map<std::string, uint64_t> entryContentUncompressedSizeTable{};

        explicit ReadOnlyArchive(const std::string &archiveFilePath);

    public:

        static ReadOnlyArchive Open(const std::string &archiveFilePath);

        [[nodiscard]] const std::map<std::string, uint64_t> &getFileContentOffsetTable() const;

        [[nodiscard]] std::unique_ptr<Stream::DataReadStream> getEntryStream(const std::string &entryName);

        uint64_t getUncompressedEntrySize(const std::string &entryName);
    };

    class WriteOnlyArchive {
    private:
        Stream::FileDataWriteStream dataStream;

        /**
         * A variable used to keep track where new entries should be placed in the heap, which is
         * the region in the archive file which starts after the entry-table.
         */
        uint64_t currentHeapOffset = 0;

        /**
         * Stores the offsets of different entries where they are stored in the heap.
         * The offset is relative from the heap start.
         */
        std::map<std::string, uint64_t> entryContentOffsetTable{};

        uint64_t heapStart = -1;

        /**
         * Stores the number of entries reserved in the archive file.
         */
        uint64_t numEntries = 0;

        bool entryTableFinalized = false;

        explicit WriteOnlyArchive(const std::string &archiveFilePath);

    public:

        static WriteOnlyArchive Open(const std::string &archiveFilePath);

        void defineEntryFromUncompressedStream(uint64_t entryIndex, const std::string &entryName,
                                               Stream::DataReadStream &uncompressedStream);

        void reserveNEntries(uint64_t numEntries);

        void finalizeEntryTable();

        void close();

    private:
        static uint64_t getEntryTableOffset(uint64_t entryIndex);
    };

}