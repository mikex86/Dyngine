#include "Dyngine/EngineState.hpp"

Dyngine::EngineState::EngineState(const std::shared_ptr<EngineRenderTarget> &engineRenderTarget,
                                  const std::shared_ptr<InputProvider> &inputProvider) :
        engineRenderTarget(engineRenderTarget),
        inputProvider(inputProvider) {

}
