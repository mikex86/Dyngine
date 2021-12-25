#pragma once

#include <Stream/AbstractDataWriteStream.hpp>
#include <fstream>
#include <string>

namespace Stream {

    NEW_EXCEPTION_TYPE(FileDataWriteStreamOpenFailedException);

    class FileDataWriteStream : public AbstractDataWriteStream {

    private:
        std::ofstream stream;
        std::string filePath;

    public:

        explicit FileDataWriteStream(const std::string &filePath);

        ~FileDataWriteStream();

        static std::unique_ptr<FileDataWriteStream> Open(const std::string &filePath);

        /**
         * Seeks to the specified position
         * @param position the new stream position
         * @throws StreamSeekException if seeking fails
         */
        void seek(uint64_t position) override;

        /**
         * Skips a specified number of bytes in the stream
         * @param offset the number of bytes to skip
         * @throws StreamSeekException if seeking fails
         */
        void skip(uint64_t offset) override;

        /**
         * Writes a byte to the file at the current stream position
         * @param uint8 the byte to write
         * @throws StreamWriteException if writing to the file fails
         */
        void writeUint8(uint8_t uint8) override;

        void close();

        std::pair<size_t, size_t> writeStreamContents(const std::shared_ptr<DataReadStream> &stream) override;

        void writeBuffer(const uint8_t *buffer, size_t size) override;

        [[nodiscard]] const std::string &getFilePath() const;
    };

}