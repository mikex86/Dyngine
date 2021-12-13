#include "PerspectiveCamera.hpp"

PerspectiveCamera::PerspectiveCamera(float fovDegrees, float aspect, float near, float far) : fov(fovDegrees),
                                                                                              aspect(aspect),
                                                                                              near(near), far(far) {
    position = glm::vec3(0.0f, 0.0f, 0.0f);
    direction = glm::vec3(0.0f, 0.0f, -1.0f);
    upAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    cameraShaderState.modelMatrix = glm::mat4(1.0f);
}

bool PerspectiveCamera::update() {

    cameraRight = glm::normalize(glm::cross(upAxis, direction));
    cameraUp = glm::normalize(glm::cross(direction, cameraRight));

    cameraShaderState.projectionMatrix = glm::perspective(glm::radians(fov), aspect, near, far);
    cameraShaderState.viewMatrix = glm::lookAt(position, position + direction, cameraUp);

    bool hasChangedSinceLastUpdate = hasChanged;
    hasChanged = false;
    return hasChangedSinceLastUpdate;
}

void PerspectiveCamera::setPosition(const glm::vec3 &cameraPosition) {
    PerspectiveCamera::position = cameraPosition;
    hasChanged = true;
}

const glm::vec3 &PerspectiveCamera::getPosition() const {
    return position;
}

const glm::vec3 &PerspectiveCamera::getDirection() const {
    return direction;
}

void PerspectiveCamera::setDirection(const glm::vec3 &direction) {
    PerspectiveCamera::direction = direction;
    hasChanged = true;
}

void PerspectiveCamera::setYaw(float yawDegrees) {
    direction.x = cos(glm::radians(yawDegrees));
    direction.z = sin(glm::radians(yawDegrees));
    hasChanged = true;
}

void PerspectiveCamera::setPitch(float pitchDegrees) {
    direction.y = sin(glm::radians(pitchDegrees));
    hasChanged = true;
}

void PerspectiveCamera::setRoll(float rollDegrees) {
    upAxis.x = cos(glm::radians(rollDegrees));
    upAxis.y = sin(glm::radians(rollDegrees));
    hasChanged = true;
}

const glm::mat4 &PerspectiveCamera::getViewMatrix() const {
    return cameraShaderState.viewMatrix;
}

void PerspectiveCamera::setAspect(float aspectRatio) {
    PerspectiveCamera::aspect = aspectRatio;
    hasChanged = true;
}

const CameraShaderState &PerspectiveCamera::getCameraShaderState() const {
    return cameraShaderState;
}

const glm::vec3 &PerspectiveCamera::getCameraRight() const {
    return cameraRight;
}

float PerspectiveCamera::getYaw() const {
    return glm::degrees(glm::atan(direction.z, direction.x));
}

float PerspectiveCamera::getPitch() const {
    return glm::degrees(glm::asin(direction.y));
}

float PerspectiveCamera::getRoll() const {
    return glm::degrees(glm::atan(upAxis.y, upAxis.x));
}

const glm::vec3 &PerspectiveCamera::getCameraUp() const {
    return cameraUp;
}
