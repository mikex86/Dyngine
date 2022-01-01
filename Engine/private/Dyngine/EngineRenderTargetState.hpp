#pragma once

#include <LLGL/Texture.h>
#include <LLGL/RenderSystem.h>
#include "Dyngine/EngineRenderContext.hpp"

namespace Dyngine {

    class FrameBufferRenderTargetState {
    public:
        std::shared_ptr<LLGL::RenderSystem> renderSystem;
        LLGL::RenderPass *renderPass{};
        LLGL::Texture *renderTargetTexture{};

        FrameBufferRenderTargetState(std::unique_ptr<EngineRenderContext> &renderSystem, uint32_t width, uint32_t height);

        virtual ~FrameBufferRenderTargetState();
    };

}