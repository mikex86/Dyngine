#pragma once

#include <memory>
#include <LLGL/LLGL.h>
#include "Dyngine/Rendering/Scene/Camera/Camera.hpp"
#include "Dyngine/Rendering/Shader/ShaderCache.hpp"
#include "Asset.hpp"
#include "Dyngine/Rendering/Scene/Asset/Node/NodeRenderer.hpp"

class AssetRenderer {
private:
    std::vector<std::unique_ptr<NodeRenderer>> nodeRenderers{};

public:
    AssetRenderer(std::vector<std::unique_ptr<NodeRenderer>> &nodeRenderers);

    static AssetRenderer *fromAsset(const std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                                    LLGL::RenderTarget *renderTarget,
                                    const std::shared_ptr<ShaderCache> &shaderCache,
                                    const std::shared_ptr<Asset> &asset,
                                    ICamera &camera,
                                    LLGL::Buffer *lightShaderStateBuffer
    );

    void render(LLGL::CommandBuffer &commandBuffer);
};