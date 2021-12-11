#include <RenderLib/RenderContext.hpp>

namespace RenderLib {

    RenderDevice::~RenderDevice() = default;

    FrameBuffer::FrameBuffer(RenderSystemBackend backend) : backend(backend) {
    }

    FrameBuffer::~FrameBuffer() = default;
}