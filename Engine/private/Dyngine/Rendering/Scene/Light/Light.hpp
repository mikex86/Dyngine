#pragma once

#include <glm/glm.hpp>

#define MAX_POINT_LIGHTS 10

enum LightType {
    POINT
};

struct LightsShaderState {
    // x, y, z = position, w = alignment dummy (because vec3 is aligned as 16 bytes), same as vec4
    glm::vec4 pointLightPositions[MAX_POINT_LIGHTS];

    // RGB = color, A = intensity
    glm::vec4 pointLightColors[MAX_POINT_LIGHTS];

    int32_t numPointLights; // Placed at the end because of spooky alignment issues. Not exactly sure why, but it works here.
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