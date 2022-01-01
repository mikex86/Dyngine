#pragma once

#include <glm/glm.hpp>

struct CameraShaderState {
    glm::mat4 viewMatrix{};
    glm::mat4 projectionMatrix{};
    glm::mat4 modelMatrix{};

    // this is placed at the end for alignment convenience. vec3 is aligned as 16 bytes in std140 buffer
    glm::vec3 position{};
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

    /**
     * Sets the camera's model matrix
     */
    virtual void setModelMatrix(const glm::mat4 &modelMatrix) = 0;

};