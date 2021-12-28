#pragma once

#include "Camera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class PerspectiveCamera : public ICamera {
private:
    float fov, aspect, near, far;
    glm::vec3 position{};
    glm::vec3 direction{};
    glm::vec3 upAxis{};
    glm::vec3 cameraRight{};
    glm::vec3 cameraUp{};
    CameraShaderState cameraShaderState{};
    bool hasChanged = true;
public:

    PerspectiveCamera(float fovDegrees, float aspect, float near, float far);

    bool update() override;

    void setPosition(const glm::vec3 &position);

    void setDirection(const glm::vec3 &direction);

    void setRotation(float yawDegrees, float pitchDegrees);

    void setRoll(float rollDegrees);

    void setAspect(float aspect);

    void setFov(float fov);

    [[nodiscard]] const glm::vec3 &getPosition() const;

    [[nodiscard]] const glm::vec3 &getDirection() const;

    [[nodiscard]] const glm::vec3 &getCameraRight() const;

    [[nodiscard]] const glm::vec3 &getCameraUp() const;

    [[nodiscard]] const glm::mat4 &getViewMatrix() const;

    const CameraShaderState &getCameraShaderState() const override;

    void setModelMatrix(const glm::mat4 &modelMatrix) override;

public:

    [[nodiscard]] float getYaw() const;

    [[nodiscard]] float getPitch() const;

    [[nodiscard]] float getRoll() const;

    float getFov() const;

};