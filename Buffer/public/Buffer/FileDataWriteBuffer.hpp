#pragma once

#include <Buffer/AbstractDataWriteBuffer.hpp>
#include <fstream>
#include <string>

namespace buffer {

    NEW_EXCEPTION_TYPE(FileDataWriteBufferOpenFailedException);

    class FileDataWriteBuffer : public AbstractDataWriteBuffer {

    private:
        std::ofstream stream;
        std::string filePath;

        explicit FileDataWriteBuffer(const std::string &filePath);

        void close();

    public:

        ~FileDataWriteBuffer();

        static FileDataWriteBuffer Open(const std::string &filePath);

        /**
         * Seeks to the specified position
         * @param position the new stream position
         * @throws BufferSeekException if seeking fails
         */
        void seek(uint64_t position) override;

        /**
         * Skips a specified number of bytes in the stream
         * @param offset the number of bytes to skip
         * @throws BufferSeekException if seeking fails
         */
        void skip(uint64_t offset) override;

        /**
         * Writes a byte to the file at the current stream position
         * @param uint8 the byte to write
         * @throws BufferWriteException if writing to the file fails
         */
        void writeUint8(uint8_t uint8) override;

        void writeStreamContents(std::ifstream &stream) override;

        const std::string &getFilePath() const;
    };

}