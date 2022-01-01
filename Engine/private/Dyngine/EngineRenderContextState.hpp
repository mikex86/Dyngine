#pragma once

#include <LLGL/LLGL.h>

namespace Dyngine {

    class EngineRenderContextState {
    public:
        std::shared_ptr<LLGL::RenderSystem> renderSystem;

        LLGL::RenderTarget *renderTarget{};
        LLGL::RenderContext *renderContext{};
        LLGL::CommandBuffer *commandBuffer{};

        EngineRenderContextState(const std::shared_ptr<LLGL::RenderSystem> &renderSystem);

        virtual ~EngineRenderContextState();
    };

}