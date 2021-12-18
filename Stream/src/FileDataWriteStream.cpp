#include <Stream/FileDataWriteStream.hpp>
#include <Stream/StreamExcept.hpp>

namespace Stream {

    EXCEPTION_TYPE_DEFAULT_IMPL(FileDataWriteStreamOpenFailedException);

    static std::ofstream OpenStream(const std::string &filePath) {
        std::ofstream stream(filePath, std::ios::binary | std::ios::out);
        if (!stream.is_open()) {
            RAISE_EXCEPTION(FileDataWriteStreamOpenFailedException,
                            "Failed to open ofstream for path: \"" + filePath + "\"");
        }
        return stream;
    }

    void FileDataWriteStream::writeUint8(uint8_t uint8) {
        stream.write(reinterpret_cast<const char *>(&uint8), 1);
        if (stream.bad()) {
            RAISE_EXCEPTION(StreamWriteException, "Failed to write to ofstream");
        }
        position++;
    }


    void FileDataWriteStream::seek(uint64_t newPosition) {
        // This must be met, or we can only open 4 GB files, which would be terrible
        static_assert(sizeof(uint64_t) <= sizeof(std::ofstream::pos_type));
        stream.seekp(newPosition);
        position = newPosition;
        if (stream.bad()) {
            RAISE_EXCEPTION(StreamSeekException,
                            "Failed to seek to " + std::to_string(newPosition) + " bytes in ofstream");
        }
    }

    void FileDataWriteStream::skip(uint64_t offset) {
        // This must be met, or we can only open 4 GB files, which would be terrible
        static_assert(sizeof(uint64_t) <= sizeof(std::ofstream::pos_type));
        stream.seekp(static_cast<std::ofstream::pos_type>(offset), std::ios::end);
        position += offset;
        if (stream.bad()) {
            RAISE_EXCEPTION(StreamSeekException, "Failed to seek " + std::to_string(offset) + " bytes in ofstream");
        }
    }


    FileDataWriteStream::FileDataWriteStream(const std::string &filePath) : AbstractDataWriteStream(-1, 0),
                                                                            stream(OpenStream(filePath)),
                                                                            filePath(filePath) {
    }

    std::shared_ptr<FileDataWriteStream> FileDataWriteStream::Open(const std::string &filePath) {
        return std::make_shared<FileDataWriteStream>(filePath);
    }

    std::pair<size_t, size_t> FileDataWriteStream::writeStreamContents(DataReadStream &inputStream) {
        size_t nWritten = 0;
        while (inputStream.hasRemaining()) {
            writeUint8(inputStream.readUint8());
            nWritten++;
        }
        return {nWritten, nWritten};
    }

    void FileDataWriteStream::writeBuffer(const uint8_t *buffer, size_t size) {
        // This must be met, or we can only open 4 GB files, which would be terrible
        static_assert(sizeof(uint64_t) <= sizeof(std::ofstream::pos_type));
        stream.write(reinterpret_cast<const char *>(buffer), static_cast<std::streamsize>(size));
        if (stream.bad()) {
            RAISE_EXCEPTION(StreamWriteException, "Failed to write to ofstream");
        }
        position += size;
    }

    void FileDataWriteStream::close() {
        if (stream.is_open())
            stream.close();
        position = 0;
    }

    FileDataWriteStream::~FileDataWriteStream() {
        close();
    }

    const std::string &FileDataWriteStream::getFilePath() const {
        return filePath;
    }

}