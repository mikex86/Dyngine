#pragma once

#include <RenderLib/RenderContext.hpp>
#include <RenderLib/FrameBuffer.hpp>
#include <memory>

namespace RenderLib {

    struct CommandBuffer {

        RenderSystemBackend backend;

        explicit CommandBuffer(RenderSystemBackend backend);

        virtual void begin() = 0;
        virtual void beginRenderPass() = 0;
        virtual void end() = 0;
        virtual void endRenderPass() = 0;
        virtual void bindGraphicsPipeline(const std::shared_ptr<GraphicsPipeline> &graphicsPipeline) = 0;

        virtual ~CommandBuffer() = default;

    };

    std::shared_ptr<RenderLib::CommandBuffer>
    CreateCommandBuffer(const std::shared_ptr<RenderLib::FrameBuffer> &frameBuffer);

}