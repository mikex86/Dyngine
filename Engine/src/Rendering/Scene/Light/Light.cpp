#include "Rendering/Scene/Light/Light.hpp"

Light::Light(LightType lightType, glm::vec3 position, glm::vec3 color, float intensity) : lightType(lightType), position(position), color(color), intensity(intensity) {
}

LightType Light::getLightType() const {
    return lightType;
}

const glm::vec3 &Light::getPosition() const {
    return position;
}

const glm::vec3 &Light::getColor() const {
    return color;
}

float Light::getIntensity() const {
    return intensity;
}

uint64_t Light::getId() const {
    return id;
}
