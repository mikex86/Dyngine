#include <Dpac/Dpac.hpp>
#include <Utils/FileUtils.hpp>
#include <Stream/ZstdDeflateStream.hpp>
#include <Stream/ZstdInflateStream.hpp>

using namespace Dpac;

EXCEPTION_TYPE_DEFAULT_IMPL(ArchiveEntryTableNotYetFinalizedException);
EXCEPTION_TYPE_DEFAULT_IMPL(ArchiveEntryTableAlreadyFinalizedException);
EXCEPTION_TYPE_DEFAULT_IMPL(ArchiveCloseFailedException);
EXCEPTION_TYPE_DEFAULT_IMPL(ArchiveOpenFailedException);
EXCEPTION_TYPE_DEFAULT_IMPL(ArchiveTooFewEntriesReservedException);
EXCEPTION_TYPE_DEFAULT_IMPL(ArchiveEntryNotDefinedException);
EXCEPTION_TYPE_DEFAULT_IMPL(EntryDoesNotExistException);

ReadOnlyArchive::ReadOnlyArchive(const std::string &archiveFilePath) : dataStream(
        Stream::FileDataReadStream::Open(archiveFilePath)) {
    heapStart = dataStream.readUint64();

    // Read entry table
    while (dataStream.getPosition() < heapStart) {
        std::string entryName = dataStream.readFixedString(DPAC_MAX_PATH);
        entryContentOffsetTable[entryName] = dataStream.readUint64();
        entryContentCompressedSizeTable[entryName] = dataStream.readUint64();
        entryContentUncompressedSizeTable[entryName] = dataStream.readUint64();
    }
}

ReadOnlyArchive ReadOnlyArchive::Open(const std::string &path) {
    return ReadOnlyArchive(path);
}

const std::map<std::string, uint64_t> &ReadOnlyArchive::getFileContentOffsetTable() const {
    return entryContentOffsetTable;
}

std::unique_ptr<Stream::DataReadStream> ReadOnlyArchive::getEntryStream(const std::string &entryName) {
    auto iterator = entryContentOffsetTable.find(entryName);
    if (iterator == entryContentOffsetTable.end()) {
        RAISE_EXCEPTION(EntryDoesNotExistException, "No entry named \"" + entryName + "\" exists in the archive");
    }
    uint64_t heapRelativeOffset = iterator->second;
    uint64_t absoluteOffset = heapStart + heapRelativeOffset;
    return std::make_unique<Stream::ZstdInflateStream>(
            std::make_shared<Stream::FileDataReadStream>(
                    dataStream.getFilePath(), absoluteOffset,
                    entryContentCompressedSizeTable[entryName]
            )
    );
}

uint64_t ReadOnlyArchive::getUncompressedEntrySize(const std::string &entryName) {
    auto iterator = entryContentUncompressedSizeTable.find(entryName);
    if (iterator == entryContentUncompressedSizeTable.end()) {
        RAISE_EXCEPTION(EntryDoesNotExistException, "No entry named \"" + entryName + "\" exists in the archive");
    }
    return iterator->second;
}

WriteOnlyArchive::WriteOnlyArchive(const std::string &archiveFilePath) :
        dataStream(Stream::FileDataWriteStream::Open(archiveFilePath)) {
    // We will seek back here on entry table finalization, which marks the beginning of the heap,
    // where all the entry contents are defined.
    // Where this heap starts will be stored as an absolute seek offset in these 8 bytes which we seek past for now.
    dataStream.skip(sizeof(uint64_t));
}

WriteOnlyArchive WriteOnlyArchive::Open(const std::string &archiveFilePath) {
    return WriteOnlyArchive(archiveFilePath);
}

uint64_t WriteOnlyArchive::getEntryTableOffset(uint64_t entryIndex) {
    // Skip heap start header (64-bit) + n * entry_size
    // entry = fixed BYTE string + 64-bit offset, + 64-bit compressed size, + 64-bit uncompressed size
    return sizeof(uint64_t) + entryIndex * (DPAC_MAX_PATH + sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint64_t));
}

void WriteOnlyArchive::defineEntryFromUncompressedStream(uint64_t entryIndex, const std::string &entryName,
                                                         Stream::DataReadStream &sourceStream) {

    if (!entryTableFinalized) {
        RAISE_EXCEPTION(ArchiveEntryTableNotYetFinalizedException,
                        "Archive entry table not finalized. Call finalizeEntryTable() before defining entries.");
    }
    if (entryIndex >= numEntries) {
        RAISE_EXCEPTION(ArchiveTooFewEntriesReservedException,
                        "Archive too few entries reserved. Needed: " +
                        std::to_string(entryIndex + 1) + ", reserved: " +
                        std::to_string(numEntries)
        );
    }
    std::string theEntryName = entryName;
    if (theEntryName.find_first_of('/') != 0) {
        theEntryName = "/" + theEntryName;
    }
    entryContentOffsetTable[theEntryName] = currentHeapOffset;

    dataStream.seek(heapStart + currentHeapOffset);
    auto zstdDataStream = Stream::ZstdDeflateStream(
            std::shared_ptr<Stream::AbstractDataWriteStream>(&dataStream, [](Stream::AbstractDataWriteStream *) {}));
    std::pair<size_t, size_t> nWrittenAndRead = zstdDataStream.writeStreamContents(sourceStream);

    auto nBytesWritten = nWrittenAndRead.first;
    auto nBytesRead = nWrittenAndRead.second;

    dataStream.seek(getEntryTableOffset(entryIndex));
    dataStream.writeFixedString(entryName, DPAC_MAX_PATH);

    // Write offset of entry content relative from heap start
    dataStream.writeUint64(currentHeapOffset);

    // Write compressed size of entry content
    dataStream.writeUint64(nBytesWritten);

    // Write uncompressed size
    dataStream.writeUint64(nBytesRead);

    currentHeapOffset += nBytesWritten;
}

void WriteOnlyArchive::reserveNEntries(uint64_t nEntries) {
    if (entryTableFinalized) {
        RAISE_EXCEPTION(ArchiveEntryTableAlreadyFinalizedException, "Entry table already finalized");
    }
    numEntries += nEntries;
}

void WriteOnlyArchive::finalizeEntryTable() {
    heapStart = getEntryTableOffset(numEntries);
    dataStream.seek(0);
    dataStream.writeUint64(heapStart);
    entryTableFinalized = true;
}

void WriteOnlyArchive::close() {
    dataStream.close();
}
