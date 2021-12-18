#include <Stream/FileDataReadStream.hpp>
#include <Utils/FileUtils.hpp>
#include <ErrorHandling/FileExcept.hpp>
#include <Stream/StreamExcept.hpp>

using namespace Stream;

#define CHECK_POSITION(neededCapacity) \
if (size != -1 && (position - startPosition) + (neededCapacity) > size)  \
RAISE_EXCEPTION(StreamUnderflowException, "Tried to read from an exhausted stream")

EXCEPTION_TYPE_DEFAULT_IMPL(FileDataReadStreamOpenFailedException);

// Handle the errors from GetFileSize
static uint64_t FileDataStreamGetFileSize(const std::string &filePath) {
    uint64_t fileSize;
    try {
        fileSize = FileUtils::GetFileSize(filePath);
    } catch (const errorhandling::FileException &e) {
        RAISE_EXCEPTION_CAUSED_BY(FileDataReadStreamOpenFailedException,
                                  std::string("Failed to open FileDataStream for path: \"") + filePath + "\"", e);
    }
    return fileSize;
}

FileDataReadStream::FileDataReadStream(const std::string &filePath, uint64_t position, uint64_t size) :
        AbstractDataReadStream(size, position),
        filePath(filePath) {
    stream = std::ifstream(filePath, std::ios::binary | std::ios::in);
    if (!stream) {
        RAISE_EXCEPTION(FileDataReadStreamOpenFailedException,
                        "Failed to open FileDataStream for path: \"" + filePath + "\": ifstream could not be opened");
    }
    seek(position);
}

FileDataReadStream::FileDataReadStream(const std::string &filePath) :
        FileDataReadStream(filePath, 0, FileDataStreamGetFileSize(filePath)) {
}

FileDataReadStream FileDataReadStream::Open(const std::string &filePath) {
    return FileDataReadStream(filePath);
}

uint8_t FileDataReadStream::readUint8() {
    CHECK_POSITION(1);
    char byte{};
    stream.read(&byte, 1);
    position++;
    return static_cast<uint8_t>(byte);
}


void FileDataReadStream::seek(uint64_t newPosition) {
    // This must be met, or we can only open 4 GB files, which would be terrible
    static_assert(sizeof(uint64_t) <= sizeof(std::ifstream::pos_type));
    if ((newPosition - startPosition) > size) {
        RAISE_EXCEPTION(StreamSeekException, "Tried to seek to position " + std::to_string(newPosition) +
                                             " in a stream of size " + std::to_string(size));
    }
    stream.seekg(static_cast<std::ifstream::pos_type>(newPosition));
    if (stream.bad()) {
        RAISE_EXCEPTION(StreamSeekException, "Failed to seek to " + std::to_string(newPosition) + " bytes in ofstream");
    }
    position = newPosition;
}

void FileDataReadStream::skip(uint64_t offset) {
    // This must be met, or we can only open 4 GB files, which would be terrible
    uint64_t newPosition = position + offset;
    if ((newPosition - startPosition) >= size) {
        RAISE_EXCEPTION(StreamSeekException, "Tried to seek to position " + std::to_string(newPosition) +
                                                  " in a stream of size " + std::to_string(size));
    }
    static_assert(sizeof(uint64_t) <= sizeof(std::ifstream::pos_type));
    stream.seekg(static_cast<std::ifstream::pos_type>(offset), std::ios::end);
    if (stream.bad()) {
        RAISE_EXCEPTION(StreamSeekException, "Failed to seek " + std::to_string(offset) + " bytes in ofstream");
    }
    position += offset;
}

void FileDataReadStream::close() {
    if (stream.is_open())
        stream.close();
    position = 0;
}

FileDataReadStream::~FileDataReadStream() {
    close();
}

const std::string &FileDataReadStream::getFilePath() const {
    return filePath;
}

bool FileDataReadStream::hasRemaining() const {
    return stream.good() && (size == -1 || (position - startPosition) < size);
}

size_t FileDataReadStream::read(uint8_t *buffer, size_t bufferLength) {
    stream.read(reinterpret_cast<char *>(buffer), bufferLength);
    position += bufferLength;
    return stream.gcount();
}
