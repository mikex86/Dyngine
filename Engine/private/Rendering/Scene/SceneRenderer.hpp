#pragma once

#include "Rendering/Scene/Asset/Asset.hpp"
#include "Rendering/Scene/Asset/AssetRenderer.hpp"
#include "Rendering/Shader/ShaderCache.hpp"
#include "Rendering/Scene/Camera/Camera.hpp"
#include "Rendering/Scene/Scene.hpp"
#include <LLGL/LLGL.h>

class SceneRenderer {

private:
    std::vector<std::unique_ptr<AssetRenderer>> renderers{};

    std::shared_ptr<LLGL::RenderSystem> renderSystem;
    LLGL::Buffer *cameraShaderStateBuffer;
    LLGL::RenderContext *renderContext;
    ShaderCache &shaderCache;
    Scene &scene;

public:
    SceneRenderer(const std::shared_ptr<LLGL::RenderSystem> &renderSystem, LLGL::RenderContext *renderContext,
                  ShaderCache &shaderCache, Scene &scene);

    void render(LLGL::CommandBuffer &commandBuffer);

private:
    void addNewAssetRenders();

    void updateLightData();

private:
    void updateCameraShaderStateBuffer(LLGL::CommandBuffer &commandBuffer);
};