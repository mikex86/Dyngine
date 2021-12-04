#include <Dpac/Dpac.hpp>
#include <Utils/FileUtils.hpp>
#include <cassert>

using namespace dpac;

EXCEPTION_TYPE_DEFAULT_IMPL(ArchiveCloseFailedException);
EXCEPTION_TYPE_DEFAULT_IMPL(ArchiveOpenFailedException);
EXCEPTION_TYPE_DEFAULT_IMPL(ArchiveFileTableAlreadyFinalizedException);
EXCEPTION_TYPE_DEFAULT_IMPL(ArchiveFileNotDefinedException);
EXCEPTION_TYPE_DEFAULT_IMPL(ArchiveFileTableNotYetFinalizedException);
EXCEPTION_TYPE_DEFAULT_IMPL(EntryDoesNotExistException);

ReadOnlyArchive::ReadOnlyArchive(const std::string &archiveFilePath) : dataBuffer(
        buffer::FileDataReadBuffer::Open(archiveFilePath)) {
    heapStart = dataBuffer.readUint64();

    // Read entry table
    uint64_t lastHeapRelativeOffset = 0;
    std::string lastEntryName;
    while (dataBuffer.getPosition() < heapStart) {
        std::string entryName = dataBuffer.readString();
        uint64_t offsetRelativeFromHeap = dataBuffer.readUint64();
        if (!lastEntryName.empty()) {
            uint64_t lastEntryContentLength = offsetRelativeFromHeap - lastHeapRelativeOffset;
            entryContentSizeTable[lastEntryName] = lastEntryContentLength;
        }
        entryContentOffsetTable[entryName] = offsetRelativeFromHeap;
        lastHeapRelativeOffset = offsetRelativeFromHeap;
        lastEntryName = entryName;
    }
    if (!lastEntryName.empty()) {
        uint64_t archiveFileSize = FileUtils::GetFileSize(archiveFilePath);
        uint64_t lastEntryContentLength = archiveFileSize - heapStart - lastHeapRelativeOffset;
        entryContentSizeTable[lastEntryName] = lastEntryContentLength;
    }
}

ReadOnlyArchive ReadOnlyArchive::Open(const std::string &path) {
    return ReadOnlyArchive(path);
}

const std::map<std::string, uint64_t> &ReadOnlyArchive::getFileContentOffsetTable() const {
    return entryContentOffsetTable;
}

buffer::FileDataReadBuffer ReadOnlyArchive::getEntryStream(const std::string &entryName) {
    auto iterator = entryContentOffsetTable.find(entryName);
    if (iterator == entryContentOffsetTable.end()) {
        RAISE_EXCEPTION(EntryDoesNotExistException, "No entry named \"" + entryName + "\" exists in the archive");
    }
    uint64_t heapRelativeOffset = iterator->second;
    uint64_t absoluteOffset = heapStart + heapRelativeOffset;
    return {dataBuffer.getFilePath(), absoluteOffset, entryContentSizeTable[entryName]};
}

WriteOnlyArchive::WriteOnlyArchive(const std::string &archiveFilePath) :
        dataBuffer(buffer::FileDataWriteBuffer::Open(archiveFilePath)) {
    // We will seek back here on entry table finalization, which marks the beginning of the heap,
    // where all the entry contents are defined.
    // Where this heap starts will be stored as an absolute seek offset in these 8 bytes which we seek past for now.
    dataBuffer.skip(sizeof(uint64_t));
}

WriteOnlyArchive WriteOnlyArchive::Open(const std::string &archiveFilePath) {
    return WriteOnlyArchive(archiveFilePath);
}

void WriteOnlyArchive::createEntry(const std::string &entryName, uint64_t contentLength) {
    if (entryTableComplete) {
        RAISE_EXCEPTION(ArchiveFileTableAlreadyFinalizedException,
                        "Cannot create any more entries. File table was already finalized.");
    }
    dataBuffer.writeString(entryName);
    dataBuffer.writeUint64(currentHeapOffset);
    entryContentOffsetTable[entryName] = currentHeapOffset;
    currentHeapOffset += contentLength;
}

void WriteOnlyArchive::finalizeEntryTable() {
    entryTableComplete = true;
    heapStart = dataBuffer.getPosition();
    dataBuffer.seek(0);
    dataBuffer.writeUint64(heapStart);
    dataBuffer.seek(heapStart);
}

void WriteOnlyArchive::createEntryContentDefinition(const std::string &entryName, std::ifstream &sourceStream) {
    if (!entryTableComplete) {
        RAISE_EXCEPTION(ArchiveFileTableNotYetFinalizedException,
                        "Cannot create content definition for a entry, when the entry-table of the archive was not yet finalized.");
    }
    auto iterator = entryContentOffsetTable.find(entryName);
    if (iterator == entryContentOffsetTable.end()) {
        RAISE_EXCEPTION(ArchiveFileNotDefinedException,
                        "Cannot define entry content for a entry which was never defined in the entry-table of the archive");
    }
    uint64_t entryOffsetFromHeap = iterator->second;
    uint64_t heapOffsetInFile = heapStart;
    assert(heapOffsetInFile != -1);

    uint64_t absoluteFileOffset = heapOffsetInFile + entryOffsetFromHeap;
    dataBuffer.seek(absoluteFileOffset);
    dataBuffer.writeStreamContents(sourceStream);
}
