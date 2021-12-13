#pragma once

#include <RenderLib/VertexFormat.hpp>
#include <RenderLib/RenderContext.hpp>
#include <memory>
#include <utility>

namespace RenderLib {

    enum BufferType {
        VERTEX_BUFFER,
        INDEX_BUFFER
    };

    struct BufferDescriptor {
        /**
         * Size of the buffer in bytes
         */
        size_t bufferSize;

        /**
         * The type of buffer object. (eg. vertex buffer)
         */
        BufferType bufferType;

        BufferDescriptor(size_t bufferSize, BufferType bufferType);
    };

    struct BufferObject {
        RenderSystemBackend backend;
        BufferType bufferType;

        BufferObject(RenderSystemBackend backend, BufferType bufferType);

        virtual ~BufferObject();

    };

    std::shared_ptr<BufferObject> CreateBufferObject(const std::shared_ptr<RenderLib::RenderContext> &renderContext, const BufferDescriptor &bufferDescriptor,
                                                     const void *data, size_t dataSize);

}