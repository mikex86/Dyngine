#pragma once

#include <glm/glm.hpp>
#include <LLGL/LLGL.h>

struct Material {
private:
    std::shared_ptr<LLGL::RenderSystem> renderSystem;

public:
    glm::vec4 albedoFactor;
    float roughnessFactor;
    float metalnessFactor;
    float ambientOcclusionFactor;
    float normalScale;

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