#pragma once

#include "Dyngine/Rendering/Scene/Asset/Asset.hpp"
#include "Dyngine/Rendering/Scene/Asset/AssetRenderer.hpp"
#include "Dyngine/Rendering/Shader/ShaderCache.hpp"
#include "Dyngine/Rendering/Scene/Camera/Camera.hpp"
#include "Dyngine/Rendering/Scene/Scene.hpp"
#include <LLGL/LLGL.h>

class SceneRenderer {

private:
    std::vector<std::unique_ptr<AssetRenderer>> renderers{};

    std::shared_ptr<LLGL::RenderSystem> renderSystem;
    LLGL::Buffer *cameraShaderStateBuffer;
    LLGL::RenderTarget *renderTarget;
    std::shared_ptr<ShaderCache> shaderCache;
    std::unique_ptr<Scene> scene;

    LightsShaderState lightsShaderState{};
    LLGL::Buffer *lightsShaderStateBuffer;

public:
    SceneRenderer(const std::shared_ptr<LLGL::RenderSystem> &renderSystem, LLGL::RenderTarget *renderTarget,
                  const std::shared_ptr<ShaderCache> &shaderCache, std::unique_ptr<Scene> scene);

    void render(LLGL::CommandBuffer &commandBuffer);

private:
    void addNewAssetRenders();

    void updateLightData(LLGL::CommandBuffer &commandBuffer);

private:
    void updateCameraShaderStateBuffer(LLGL::CommandBuffer &commandBuffer);
};