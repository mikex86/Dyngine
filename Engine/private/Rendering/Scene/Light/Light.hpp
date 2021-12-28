#pragma once

#include <glm/glm.hpp>

enum LightType {
    POINT
};

class Light {

private:
    LightType lightType;
    glm::vec3 position;
    glm::vec3 color;
    float intensity;

public:
    uint64_t id{};

    Light(LightType lightType, glm::vec3 position, glm::vec3 color, float intensity);

    LightType getLightType() const;

    const glm::vec3 &getPosition() const;

    const glm::vec3 &getColor() const;

    float getIntensity() const;

    uint64_t getId() const;

};