#include <Buffer/FileDataWriteBuffer.hpp>
#include <Buffer/BufferExcept.hpp>

namespace buffer {

    EXCEPTION_TYPE_DEFAULT_IMPL(FileDataWriteBufferOpenFailedException);

    static std::ofstream OpenStream(const std::string &filePath) {
        std::ofstream stream(filePath, std::ios::binary | std::ios::out);
        if (!stream.is_open()) {
            RAISE_EXCEPTION(FileDataWriteBufferOpenFailedException,
                            "Failed to open ofstream for path: \"" + filePath + "\"");
        }
        return stream;
    }

    void FileDataWriteBuffer::writeUint8(uint8_t uint8) {
        stream.write(reinterpret_cast<const char *>(&uint8), 1);
        if (stream.bad()) {
            RAISE_EXCEPTION(BufferWriteException, "Failed to write to ofstream");
        }
        position++;
    }


    void FileDataWriteBuffer::seek(uint64_t newPosition) {
        // This must be met, or we can only open 4 GB files, which would be terrible
        static_assert(sizeof(uint64_t) <= sizeof(std::ofstream::pos_type));
        stream.seekp(newPosition);
        position = newPosition;
        if (stream.bad()) {
            RAISE_EXCEPTION(BufferSeekException,
                            "Failed to seek to " + std::to_string(newPosition) + " bytes in ofstream");
        }
    }

    void FileDataWriteBuffer::skip(uint64_t offset) {
        // This must be met, or we can only open 4 GB files, which would be terrible
        static_assert(sizeof(uint64_t) <= sizeof(std::ofstream::pos_type));
        stream.seekp(offset, std::ios::end);
        position += offset;
        if (stream.bad()) {
            RAISE_EXCEPTION(BufferSeekException, "Failed to seek " + std::to_string(offset) + " bytes in ofstream");
        }
    }


    FileDataWriteBuffer::FileDataWriteBuffer(const std::string &filePath) : AbstractDataWriteBuffer(-1, 0),
                                                                            stream(OpenStream(filePath)),
                                                                            filePath(filePath) {
    }

    FileDataWriteBuffer FileDataWriteBuffer::Open(const std::string &filePath) {
        return FileDataWriteBuffer(filePath);
    }

    void FileDataWriteBuffer::writeStreamContents(std::ifstream &streamIn) {
        stream << streamIn.rdbuf();
        position = stream.tellp();
    }

    void FileDataWriteBuffer::close() {
        if (stream.is_open())
            stream.close();
        position = 0;
    }

    FileDataWriteBuffer::~FileDataWriteBuffer() {
        close();
    }

    const std::string &FileDataWriteBuffer::getFilePath() const {
        return filePath;
    }

}