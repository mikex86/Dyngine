#pragma once

#include <Stream/AbstractDataWriteStream.hpp>
#include <memory>

namespace Stream {

    class ZstdDeflateStream : public AbstractDataWriteStream {

    private:
        std::shared_ptr<AbstractDataWriteStream> sink;
        void *cCtx{}; // ZSTD_CCtx *
        uint8_t *inputBuffer;
        size_t inputBufferCapacity{};
        uint8_t *outputBuffer;
        size_t outputBufferCapacity{};
        size_t outputBufferLength{};
        size_t outputBufferReadIndex{};

    public:
        explicit ZstdDeflateStream(std::shared_ptr<AbstractDataWriteStream> sink);

    public:
        void seek(uint64_t position) override;

        void skip(uint64_t offset) override;

        void writeUint8(uint8_t uint8) override;

        std::pair<size_t, size_t> writeStreamContents(Stream::DataReadStream &stream) override;

        virtual ~ZstdDeflateStream();
    };

}