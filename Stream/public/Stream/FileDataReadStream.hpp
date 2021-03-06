#pragma once

#include <Stream/AbstractDataReadStream.hpp>
#include <ErrorHandling/ErrorHandling.hpp>
#include <fstream>
#include <string>

namespace Stream {

    NEW_EXCEPTION_TYPE(FileDataReadStreamOpenFailedException);

    class FileDataReadStream : public AbstractDataReadStream {

    private:
        std::ifstream stream;
        std::string filePath;
        uint8_t *buffer;
        int64_t bufferReadingIndex{};
        size_t bufferLength{};
        size_t bufferCapacity = READ_BUFFER_SIZE;

        void close();

    public:

        FileDataReadStream(const std::string &filePath, uint64_t position, uint64_t size);

        ~FileDataReadStream() override;

        static std::unique_ptr<FileDataReadStream> Open(const std::string &filePath);

        uint8_t readUint8() override;

        size_t read(uint8_t *buffer, size_t bufferLength) override;

        void seek(uint64_t position) override;

        void skip(uint64_t offset) override;

        [[nodiscard]] const std::string &getFilePath() const;

        [[nodiscard]] bool hasRemaining() const override;

        explicit FileDataReadStream(const std::string &filePath);
    };
}