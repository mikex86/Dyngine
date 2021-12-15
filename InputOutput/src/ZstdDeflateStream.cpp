#include <Stream/ZstdDeflateStream.hpp>
#include <ErrorHandling/IllegalStateException.hpp>
#include <zstd.h>
#include <utility>

#define CHECK_POSITION(neededCapacity) \
if (size != -1 && position + (neededCapacity) > size)  \
RAISE_EXCEPTION(StreamOverflowException, "Tried to write to an exhausted stream")

Stream::ZstdDeflateStream::ZstdDeflateStream(std::shared_ptr<AbstractDataWriteStream> sink) :
        AbstractDataWriteStream(-1, 0),
        sink(std::move(sink)) {
    auto cCtx = ZSTD_createCCtx();
    if (cCtx == nullptr) {
        RAISE_EXCEPTION(errorhandling::IllegalStateException,
                        "Failed to create ZstdDeflateStream: ZSTD_createCCtx returned null");
    }
    ZSTD_CCtx_setParameter(cCtx, ZSTD_c_compressionLevel, 0);
    ZSTD_CCtx_setParameter(cCtx, ZSTD_c_checksumFlag, 1);

    this->cCtx = cCtx;
    inputBufferCapacity = ZSTD_DStreamInSize();
    inputBuffer = new uint8_t[inputBufferCapacity];

    outputBufferCapacity = ZSTD_DStreamOutSize();
    outputBuffer = new uint8_t[outputBufferCapacity];
}

void Stream::ZstdDeflateStream::writeUint8(uint8_t uint8) {
    if (outputBufferReadIndex >= outputBufferLength || outputBufferReadIndex >= outputBufferCapacity) {
        outputBufferReadIndex = 0;
    }
    if (outputBufferReadIndex == 0) {
        bool lastChunk = sink->getPosition() + inputBufferCapacity >= sink->getSize();
        ZSTD_EndDirective mode = lastChunk ? ZSTD_e_end : ZSTD_e_continue;
        ZSTD_inBuffer input = {inputBuffer, inputBufferCapacity, 0};
        ZSTD_outBuffer output = {outputBuffer, outputBufferCapacity, 0};
        size_t result = ZSTD_compressStream2(reinterpret_cast<ZSTD_CCtx *>(cCtx), &output, &input, mode);
        if (ZSTD_isError(result)) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException,
                            "Failed to compress data: ZSTD_compressStream2 returned error");
        }
        outputBufferLength = output.pos;
        outputBufferReadIndex = 0;
    }
    position++;
    auto currentOutputBufferReadIndex = outputBufferReadIndex;
    outputBufferReadIndex++;
    auto compressedByte = outputBuffer[currentOutputBufferReadIndex];
    sink->writeUint8(compressedByte);
}

std::pair<size_t, size_t> Stream::ZstdDeflateStream::writeStreamContents(Stream::DataReadStream &stream) {
    static_assert(sizeof(std::streamsize) >= sizeof(size_t), "std::streamsize is smaller than size_t");
    size_t bytesReadTotal = 0;
    while (stream.hasRemaining()) {
        size_t readBytesInChunk = stream.read(inputBuffer, static_cast<std::streamsize>(inputBufferCapacity));
        bool lastChunk = readBytesInChunk < inputBufferCapacity;
        ZSTD_EndDirective mode = lastChunk ? ZSTD_e_end : ZSTD_e_continue;
        ZSTD_inBuffer input = {inputBuffer, readBytesInChunk, 0};
        bool finished;
        do {
            ZSTD_outBuffer output = {outputBuffer, outputBufferCapacity, 0};
            size_t remaining = ZSTD_compressStream2(reinterpret_cast<ZSTD_CCtx *>(cCtx), &output, &input, mode);
            if (ZSTD_isError(remaining)) {
                RAISE_EXCEPTION(errorhandling::IllegalStateException,
                                "Failed to compress data: ZSTD_compressStream2 returned error");
            }
            outputBufferLength = output.pos;
            sink->writeBuffer(outputBuffer, outputBufferLength);
            outputBufferReadIndex = outputBufferLength;
            position += outputBufferLength;
            finished = lastChunk ? (remaining == 0) : (input.pos == input.size);
        } while (!finished);
        bytesReadTotal += readBytesInChunk;
    }
    return {outputBufferReadIndex, bytesReadTotal};
}

void Stream::ZstdDeflateStream::seek(uint64_t position) {
    RAISE_EXCEPTION(errorhandling::IllegalStateException, "Seeking not supported for ZstdDeflateStream");
}

void Stream::ZstdDeflateStream::skip(uint64_t offset) {
    RAISE_EXCEPTION(errorhandling::IllegalStateException, "Skipping not supported for ZstdDeflateStream");
}

Stream::ZstdDeflateStream::~ZstdDeflateStream() {
    ZSTD_freeCCtx(reinterpret_cast<ZSTD_CCtx *>(cCtx));
    delete[] inputBuffer;
    delete[] outputBuffer;
}