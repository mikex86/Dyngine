#include <RenderLib/BufferObject.hpp>

namespace RenderLib {

    BufferDescriptor::BufferDescriptor(size_t bufferSize, BufferType bufferType) : bufferSize(
            bufferSize), bufferType(bufferType) {
    }

    BufferObject::BufferObject(RenderSystemBackend backend, BufferType bufferType) : backend(backend),
                                                                                     bufferType(bufferType) {}

    BufferObject::~BufferObject() = default;
}