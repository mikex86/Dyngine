#include <Buffer/FileDataReadBuffer.hpp>
#include <Utils/FileUtils.hpp>
#include <ErrorHandling/FileExcept.hpp>
#include <Buffer/BufferExcept.hpp>

using namespace buffer;

EXCEPTION_TYPE_DEFAULT_IMPL(FileDataReadBufferOpenFailedException);

// Handle the errors from GetFileSize
static uint64_t FileDataBufferGetFileSize(const std::string &filePath) {
    uint64_t fileSize;
    try {
        fileSize = FileUtils::GetFileSize(filePath);
    } catch (const errorhandling::FileException &e) {
        RAISE_EXCEPTION_CAUSED_BY(FileDataReadBufferOpenFailedException,
                                  std::string("Failed to open FileDataBuffer for path: \"") + filePath + "\"", e);
    }
    return fileSize;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "VirtualCallInCtorOrDtor"
FileDataReadBuffer::FileDataReadBuffer(const std::string &filePath, uint64_t position, uint64_t size) :
        AbstractDataReadBuffer(size, position),
        filePath(filePath) {
    stream = std::ifstream(filePath, std::ios::binary | std::ios::in);
    if (!stream) {
        RAISE_EXCEPTION(FileDataReadBufferOpenFailedException,
                        "Failed to open FileDataBuffer for path: \"" + filePath + "\": ifstream could not be opened");
    }
    seek(position);
}
#pragma clang diagnostic pop

FileDataReadBuffer::FileDataReadBuffer(const std::string &filePath) :
        FileDataReadBuffer(filePath, 0, FileDataBufferGetFileSize(filePath)) {
}

FileDataReadBuffer FileDataReadBuffer::Open(const std::string &filePath) {
    return FileDataReadBuffer(filePath);
}

uint8_t FileDataReadBuffer::readUint8() {
    char byte{};
    stream.read(&byte, 1);
    position++;
    return static_cast<uint8_t>(byte);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"

void FileDataReadBuffer::seek(uint64_t newPosition) {
    // This must be met, or we can only open 4 GB files, which would be terrible
    static_assert(sizeof(uint64_t) <= sizeof(std::ifstream::pos_type));
    stream.seekg(static_cast<std::ifstream::pos_type>(newPosition));
    if (stream.bad()) {
        RAISE_EXCEPTION(BufferSeekException, "Failed to seek to " + std::to_string(newPosition) + " bytes in ofstream");
    }
    position = newPosition;
}

void FileDataReadBuffer::skip(uint64_t offset) {
    // This must be met, or we can only open 4 GB files, which would be terrible
    static_assert(sizeof(uint64_t) <= sizeof(std::ifstream::pos_type));
    stream.seekg(static_cast<std::ifstream::pos_type>(offset), std::ios::end);
    if (stream.bad()) {
        RAISE_EXCEPTION(BufferSeekException, "Failed to seek " + std::to_string(offset) + " bytes in ofstream");
    }
    position += offset;
}

#pragma clang diagnostic pop

void FileDataReadBuffer::close() {
    if (stream.is_open())
        stream.close();
    position = 0;
}

FileDataReadBuffer::~FileDataReadBuffer() {
    close();
}

const std::string &FileDataReadBuffer::getFilePath() const {
    return filePath;
}
