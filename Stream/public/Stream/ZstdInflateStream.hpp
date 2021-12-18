#pragma once

#include <Stream/AbstractDataReadStream.hpp>
#include <memory>

namespace Stream {

    class ZstdInflateStream : public AbstractDataReadStream {

    private:
        std::shared_ptr<AbstractDataReadStream> source;
        // Using void* here is a bit of a hack, but is necessary to not expose zstd
        void *dCtx{}; // ZSTD_DCtx *
        uint8_t *inputBuffer;
        size_t inputBufferCapacity{};
        size_t inputBufferReadIndex{};
        size_t inputBufferLength{};
        uint8_t *outputBuffer;
        size_t outputBufferCapacity{};
        size_t outputBufferLength{};
        size_t outputBufferReadIndex{};

    public:
        explicit ZstdInflateStream(std::shared_ptr<AbstractDataReadStream> source);

    public:
        uint8_t readUint8() override;

        void seek(uint64_t position) override;

        void skip(uint64_t offset) override;

        ~ZstdInflateStream() override;

        [[nodiscard]] bool hasRemaining() const override;

    };

}