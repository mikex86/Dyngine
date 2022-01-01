#pragma once

#include <memory>
#include "Dyngine/Input/Input.hpp"
#include "Dyngine/Rendering/Scene/Camera/PerspectiveCamera.hpp"

// TODO: DEBUG ONLY
class FlyingPerspectiveCameraController {

private:
    PerspectiveCamera &camera;
    std::unique_ptr<Dyngine::Input> input;

public:
    FlyingPerspectiveCameraController(PerspectiveCamera &camera,
                                      std::unique_ptr<Dyngine::Input> &input);

    void update(float deltaTime);

private:
    float applyDeadZone(float value, float deadZone);
};