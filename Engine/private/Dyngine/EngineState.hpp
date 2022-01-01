#pragma once

#include <LLGL/LLGL.h>
#include "Dyngine/Rendering/Scene/Camera/PerspectiveCamera.hpp"
#include "Dyngine/Rendering/Scene/SceneRenderer.hpp"
#include "Dyngine/Rendering/Scene/Camera/Controller/FlyingPerspectiveCameraController.hpp"
#include "Dyngine/Input/InputProvider.hpp"
#include "Dyngine/EngineRenderTarget.hpp"

namespace Dyngine {

    class EngineState {
    public:
        float aspectRatio{};
        std::shared_ptr<EngineRenderTarget> engineRenderTarget;

        std::unique_ptr<LLGL::Timer> frameTimer = LLGL::Timer::Create();
        std::shared_ptr<InputProvider> inputProvider;

        EngineState(const std::shared_ptr<EngineRenderTarget> &engineRenderTarget,
                    const std::shared_ptr<InputProvider> &inputProvider);


        // TODO: DEBUG ONLY
        std::unique_ptr<PerspectiveCamera> camera{};
        std::unique_ptr<FlyingPerspectiveCameraController> cameraController{};
        std::unique_ptr<SceneRenderer> sceneRenderer{};
    };

}