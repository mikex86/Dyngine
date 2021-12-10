#include <RenderLib/FrameBuffer.hpp>

namespace RenderLib {

    FrameBuffer::FrameBuffer(RenderSystemBackend backend) : backend(backend) {
    }

    FrameBuffer::~FrameBuffer() = default;

}