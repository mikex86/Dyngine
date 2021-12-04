#pragma once

#include <ErrorHandling/ErrorHandling.hpp>
#include <Buffer/FileDataReadBuffer.hpp>
#include <Buffer/FileDataWriteBuffer.hpp>
#include <map>
#include <vector>
#include <string>

namespace dpac {

    NEW_EXCEPTION_TYPE(ArchiveOpenFailedException);
    NEW_EXCEPTION_TYPE(ArchiveCloseFailedException);
    NEW_EXCEPTION_TYPE(ArchiveFileTableAlreadyFinalizedException);
    NEW_EXCEPTION_TYPE(ArchiveFileNotDefinedException);
    NEW_EXCEPTION_TYPE(ArchiveFileTableNotYetFinalizedException);
    NEW_EXCEPTION_TYPE(EntryDoesNotExistException);

    class ReadOnlyArchive {
    private:
        buffer::FileDataReadBuffer dataBuffer;

        uint64_t heapStart{};

        /**
         * Stores the offsets of different entries where they are stored in the heap.
         * The offset is relative from the heap start.
         */
        std::map<std::string, uint64_t> entryContentOffsetTable{};

        /**
         * Stores the sizes of all entries
         */
        std::map<std::string, uint64_t > entryContentSizeTable;

        explicit ReadOnlyArchive(const std::string &archiveFilePath);

    public:

        static ReadOnlyArchive Open(const std::string &archiveFilePath);

        [[nodiscard]] const std::map<std::string, uint64_t> &getFileContentOffsetTable() const;

        [[nodiscard]] buffer::FileDataReadBuffer getEntryStream(const std::string &entryName);
    };

    class WriteOnlyArchive {
    private:
        buffer::FileDataWriteBuffer dataBuffer;

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

        bool entryTableComplete = false;

        uint64_t heapStart = -1;

        explicit WriteOnlyArchive(const std::string &archiveFilePath);

    public:

        static WriteOnlyArchive Open(const std::string &archiveFilePath);

        void createEntry(const std::string &entryName, uint64_t contentLength);

        void finalizeEntryTable();

        void createEntryContentDefinition(const std::string &entryName, std::ifstream &sourceStream);
    };

}