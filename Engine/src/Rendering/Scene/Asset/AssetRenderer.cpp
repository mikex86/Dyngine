#include "Rendering/Scene/Asset/AssetRenderer.hpp"

AssetRenderer::AssetRenderer(std::vector<std::unique_ptr<NodeRenderer>> &nodeRenderers) : nodeRenderers(std::move(nodeRenderers)) {
}

AssetRenderer *
AssetRenderer::fromAsset(const std::shared_ptr<LLGL::RenderSystem> &renderSystem, LLGL::RenderContext *renderContext,
                         ShaderCache &shaderCache, const std::shared_ptr<Asset> &asset, ICamera &camera) {
    std::vector<std::unique_ptr<NodeRenderer>> nodeRenderers{};
    for (auto &node: asset->nodes) {
        nodeRenderers.push_back(std::unique_ptr<NodeRenderer>(NodeRenderer::fromNode(renderSystem, renderContext, shaderCache, asset, camera, node)));
    }
    return new AssetRenderer(nodeRenderers);
}


void AssetRenderer::render(LLGL::CommandBuffer &commandBuffer) {
    for (auto &renderer: nodeRenderers) {
        renderer->render(commandBuffer);
    }
}
