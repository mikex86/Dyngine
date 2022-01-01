#pragma once

#include <glm/glm.hpp>
#include <LLGL/LLGL.h>

struct MaterialShaderState {
    uint32_t texturePresentStates;
    glm::vec4 albedoFactor;
    float roughnessFactor;
    float metalnessFactor;
    float ambientOcclusionFactor;
    float normalScale;
};

struct Material {
private:
    std::shared_ptr<LLGL::RenderSystem> renderSystem;

public:
    std::string name;
    glm::vec4 albedoFactor;
    float roughnessFactor;
    float metalnessFactor;
    float ambientOcclusionFactor;
    float normalScale;

    /**
     * A buffer containing an integer value containing flags that state whether a given texture is set in the material.
     * bit (1 << 0): albedo texture
     * bit (1 << 1): normal texture
     * bit (1 << 2): rma texture
     */
    LLGL::Buffer *texturePresentFlagsBuffer;

    /// Textures

    // nullable
    LLGL::Texture *albedoTexture;

    // nullable
    LLGL::Texture *normalTexture;

    /**
     * Roughness Metalness Ambient Occlusion
     */
    // nullable
    LLGL::Texture *rmaTexture;

    /// Samplers

    // nullable
    LLGL::Sampler *albedoSampler;

    // nullable
    LLGL::Sampler *normalSampler;

    // nullable
    LLGL::Sampler *rmaSampler;

    Material(const std::shared_ptr<LLGL::RenderSystem> &renderSystem);

    virtual ~Material();

};