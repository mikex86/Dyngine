#pragma once

#include <RenderLib/GraphicsPipeline.hpp>
#include <memory>

namespace RenderLib {

    struct FrameBuffer {
        RenderSystemBackend backend;

        explicit FrameBuffer(RenderSystemBackend backend);

        virtual ~FrameBuffer();
    };

    std::shared_ptr<FrameBuffer> CreateFrameBuffer(const std::shared_ptr<RenderLib::RenderContext> &renderContext);

}