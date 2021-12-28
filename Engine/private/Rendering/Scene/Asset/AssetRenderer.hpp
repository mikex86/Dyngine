#pragma once

#include <memory>
#include <LLGL/LLGL.h>
#include "Rendering/Scene/Camera/Camera.hpp"
#include "Rendering/Shader/ShaderCache.hpp"
#include "Rendering/Scene/Asset/Asset.hpp"
#include "Rendering/Scene/Asset/Node/NodeRenderer.hpp"

class AssetRenderer {
private:
    std::vector<std::unique_ptr<NodeRenderer>> nodeRenderers{};

public:
    AssetRenderer(std::vector<std::unique_ptr<NodeRenderer>> &nodeRenderers);

    static AssetRenderer *fromAsset(const std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                                    LLGL::RenderContext *renderContext,
                                    ShaderCache &shaderCache,
                                    const std::shared_ptr<Asset> &asset,
                                    ICamera &camera
    );

    void render(LLGL::CommandBuffer &commandBuffer);
};