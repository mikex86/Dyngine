#include "Dyngine/EngineRenderContextState.hpp"


Dyngine::EngineRenderContextState::EngineRenderContextState(const std::shared_ptr<LLGL::RenderSystem> &renderSystem) :
        renderSystem(renderSystem) {
}

Dyngine::EngineRenderContextState::~EngineRenderContextState() {
}
