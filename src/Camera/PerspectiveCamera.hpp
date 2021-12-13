#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct CameraShaderState {
    glm::mat4 viewMatrix{};
    glm::mat4 projectionMatrix{};
    glm::mat4 modelMatrix{};
};

class PerspectiveCamera {
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

    bool update();

    void setPosition(const glm::vec3 &position);

    void setDirection(const glm::vec3 &direction);

    void setYaw(float yawDegrees);

    void setPitch(float pitchDegrees);

    void setRoll(float rollDegrees);

    void setAspect(float aspect);

    [[nodiscard]] const glm::vec3 &getPosition() const;

    [[nodiscard]] const glm::vec3 &getDirection() const;

    [[nodiscard]] const glm::vec3 &getCameraRight() const;

    [[nodiscard]] const glm::vec3 &getCameraUp() const;

    [[nodiscard]] const glm::mat4 &getViewMatrix() const;

    [[nodiscard]] const CameraShaderState &getCameraShaderState() const;

    [[nodiscard]] float getYaw() const;

    [[nodiscard]] float getPitch() const;

    [[nodiscard]] float getRoll() const;

};