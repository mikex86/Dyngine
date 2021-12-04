#pragma once

#include <Buffer/AbstractDataReadBuffer.hpp>
#include <ErrorHandling/ErrorHandling.hpp>
#include <fstream>
#include <string>

namespace buffer {

    NEW_EXCEPTION_TYPE(FileDataReadBufferOpenFailedException);

    class FileDataReadBuffer : public AbstractDataReadBuffer {

    private:
        std::ifstream stream;
        std::string filePath;

        explicit FileDataReadBuffer(const std::string &filePath);

        void close();

    public:

        FileDataReadBuffer(const std::string &filePath, uint64_t position, uint64_t size);

        ~FileDataReadBuffer();

        static FileDataReadBuffer Open(const std::string &filePath);

        uint8_t readUint8() override;

        void seek(uint64_t position) override;

        void skip(uint64_t offset) override;

        [[nodiscard]] const std::string &getFilePath() const;
    };
}