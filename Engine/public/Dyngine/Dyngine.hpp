#pragma once

#include <memory>
#include <Dyngine/EngineRenderTarget.hpp>
#include "Dyngine/Input/InputProvider.hpp"

#define ENGINE_NAME "Dyngine"
#define ENGINE_VERSION "v0.1"


namespace Dyngine {

    class EngineState;

    class EngineInstance {
    private:
        EngineState *engineState;
    public:

        explicit EngineInstance(const std::shared_ptr<EngineRenderTarget> &renderTarget,
                                const std::shared_ptr<InputProvider> &inputProvider);

        void startEngine();

        void renderFrame();

        virtual ~EngineInstance();

        int getFPS();
    };

}