#pragma once

#include <memory>

namespace Dyngine {

    class EngineRenderContextState;

    class EngineRenderContext {
    private:
        EngineRenderContextState *renderContextState;
    public:
        EngineRenderContext();

        EngineRenderContextState *getRenderContextState();

        virtual ~EngineRenderContext();
    };

}