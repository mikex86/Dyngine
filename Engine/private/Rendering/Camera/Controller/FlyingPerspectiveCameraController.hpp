#pragma once

#include <memory>
#include "LLGL/Input.h"
#include "Rendering/Camera/PerspectiveCamera.hpp"

// TODO: DEBUG ONLY
class FlyingPerspectiveCameraController {

private:
    PerspectiveCamera &camera;

    std::shared_ptr<LLGL::Input> input;
    std::shared_ptr<LLGL::Display> display;
    std::shared_ptr<LLGL::Window> window;

public:
    FlyingPerspectiveCameraController(PerspectiveCamera &camera, std::shared_ptr<LLGL::Input> input,
                                      std::shared_ptr<LLGL::Display> display,
                                      std::shared_ptr<LLGL::Window> window);

    void update(float deltaTime);

private:
    float applyDeadZone(float value, float deadZone);
};