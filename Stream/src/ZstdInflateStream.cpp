#include <Stream/ZstdInflateStream.hpp>
#include <ErrorHandling/IllegalStateException.hpp>
#include <utility>
#include <zstd.h>

namespace Stream {

    ZstdInflateStream::ZstdInflateStream(std::shared_ptr<AbstractDataReadStream> source)
            : AbstractDataReadStream(-1, 0), source(std::move(source)) {
        dCtx = ZSTD_createDCtx();
        if (dCtx == nullptr) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException,
                            "Failed to create ZstdInflateStream: ZSTD_createDCtx returned null");
        }
        inputBufferCapacity = READ_BUFFER_SIZE;
        inputBuffer = new uint8_t[inputBufferCapacity];

        outputBufferCapacity = READ_BUFFER_SIZE;
        outputBuffer = new uint8_t[outputBufferCapacity];
    }

    // This method uses two buffers and accommodating variables:
    // The inputBuffer.
    //     This buffer is used to store the compressed data. We populate it by reading from the source stream.
    // The outputBuffer.
    //     This buffer is used to store the decompressed data. We write to it by calling ZSTD_decompressStream.
    //
    // The capacity variable of the respective buffer is the amount of memory allocated for the buffer,
    // while the length is the amount of data of that buffer, which is meaningfully populated.
    uint8_t ZstdInflateStream::readUint8() {
        // This checks whether the input buffer is empty, or all the compressed data from the outputBuffer
        // has been read and passed to the user that is streaming this stream.
        // Whenever this is the case, we need to read new compressed data from the source stream,
        // and populate the input buffer with it.
        // outputBufferReadIndex being zero is thus the state we will use to determine
        // the necessity to read new compressed data from the source stream.
        // the outputBufferReadIndex is either zero because of its initialization,
        // when we have never accessed this stream, which also requires us to read new input data,
        // as well as when the output buffer runs out.
        if (inputBufferLength == 0 || outputBufferReadIndex >= outputBufferLength) {
            outputBufferReadIndex = 0;
        }
        if (outputBufferReadIndex == 0) {
            ZSTD_inBuffer input;
            if (inputBufferReadIndex == 0) {
                auto read = source->read(inputBuffer, inputBufferCapacity);
                if (read == 0) {
                    RAISE_EXCEPTION(errorhandling::IllegalStateException,
                                    "Failed to read from ZstdInflateStream: read returned 0");
                }
                inputBufferLength = read;
            }
            input = {inputBuffer, inputBufferLength, inputBufferReadIndex};
            ZSTD_outBuffer output = {outputBuffer, outputBufferCapacity, 0};
            while (input.pos < input.size) {
                size_t ret = ZSTD_decompressStream(reinterpret_cast<ZSTD_DCtx *>(dCtx), &output, &input);
                if (output.pos == output.size && input.pos < input.size) {
                    // This is where we land, if the output buffer is too small and there is still data
                    // to decompress in the input buffer.
                    // While this means that the compression is working very well,
                    // we have to do some trickery to avoid resizing the output buffer.
                    // The next time the outputBuffer runs out, we will not read from the source stream
                    // into the input buffer - instead, we will just continue decompressing the
                    // existing inputBuffer populated by this invocation of the method
                    // starting at the inputBufferReadIndex.
                    inputBufferReadIndex = input.pos;
                    goto outputBufferTooSmall;
                }
                if (ZSTD_isError(ret)) {
                    RAISE_EXCEPTION(errorhandling::IllegalStateException,
                                    "Failed to decompress ZstdInflateStream: ZSTD_decompressStream returned " +
                                    std::to_string(ret)
                    );
                }
            }
            inputBufferReadIndex = 0;


            // We jump to this mark to prevent inputBufferReadIndex being set to 0 without needing an
            // if statement with a comparison whe just performed as second ago (output.pos == output.size).
            outputBufferTooSmall:
            outputBufferLength = output.pos;
        }
        auto currentOutputBufferReadIndex = outputBufferReadIndex;
        outputBufferReadIndex++;
        position++;
        return outputBuffer[currentOutputBufferReadIndex];
    }

    void ZstdInflateStream::seek(uint64_t position) {
        RAISE_EXCEPTION(errorhandling::IllegalStateException,
                        "Failed to seek in ZstdInflateStream: seek is not supported"
        );
    }

    void ZstdInflateStream::skip(uint64_t offset) {
        RAISE_EXCEPTION(errorhandling::IllegalStateException,
                        "Failed to skip in ZstdInflateStream: skip is not supported"
        );
    }

    ZstdInflateStream::~ZstdInflateStream() {
        ZSTD_freeDCtx(reinterpret_cast<ZSTD_DCtx *>(dCtx));
        delete[] inputBuffer;
        delete[] outputBuffer;
    }

    bool ZstdInflateStream::hasRemaining() const {
        return source->hasRemaining() || outputBufferReadIndex < outputBufferLength;
    }
}