#pragma once

#include <RenderLib/RenderContext.hpp>
#include <RenderLib/GraphicsPipeline.hpp>
#include <RenderLib/BufferObject.hpp>
#include <memory>

namespace RenderLib {

    struct CommandBuffer {

        RenderSystemBackend backend;

        explicit CommandBuffer(RenderSystemBackend backend);

        virtual void begin() = 0;

        virtual void beginRenderPass() = 0;

        virtual void end() = 0;

        virtual void endRenderPass() = 0;

        virtual void bindGraphicsPipeline(const std::shared_ptr<RenderLib::GraphicsPipeline> &graphicsPipeline) = 0;

        virtual void bindBufferObject(const std::shared_ptr<RenderLib::BufferObject> &bufferObject) = 0;

        virtual void
        draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;

        virtual ~CommandBuffer() = default;

    };

    std::shared_ptr<RenderLib::CommandBuffer>
    CreateCommandBuffer(const std::shared_ptr<RenderLib::RenderContext> &renderContext);

}