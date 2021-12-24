#pragma once

#include <glm/mat4x4.hpp>

struct CameraShaderState {
    glm::mat4 viewMatrix{};
    glm::mat4 projectionMatrix{};
    glm::mat4 modelMatrix{};
};

class ICamera {

public:
    /**
     * Updates the cameras state and returns whether the camera state has changed since last the invocation of update.
     */
    virtual bool update() = 0;

    /**
     * returns a CameraShaderState struct containing the current camera state ready to be passed to a shader.
     */
    [[nodiscard]] virtual const CameraShaderState &getCameraShaderState() const = 0;

};