#pragma once

#include <memory>
#include <LLGL/LLGL.h>
#include <Shader/ShaderCache.hpp>
#include <Asset/Asset.hpp>
#include <Mesh/MeshRenderer.hpp>

class AssetRenderer {
private:
    std::vector<std::unique_ptr<MeshRenderer>> renderers{};

public:
    AssetRenderer(std::vector<std::unique_ptr<MeshRenderer>> &renderers);

    static AssetRenderer *fromAsset(std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                                  LLGL::RenderContext *renderContext,
                                  ShaderCache &shaderCache,
                                  std::unique_ptr<Asset> &asset,
                                  ICamera &camera
    );

    void render(LLGL::CommandBuffer &commandBuffer);
};