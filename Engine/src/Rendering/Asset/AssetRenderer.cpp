#include "Rendering/Asset/AssetRenderer.hpp"

AssetRenderer::AssetRenderer(std::vector<std::unique_ptr<MeshRenderer>> &renderers) {
    for (auto &renderer: renderers) {
        this->renderers.push_back(std::move(renderer));
    }
}

AssetRenderer *
AssetRenderer::fromAsset(std::shared_ptr<LLGL::RenderSystem> &renderSystem, LLGL::RenderContext *renderContext,
                         ShaderCache &shaderCache, std::unique_ptr<Asset> &asset, ICamera &camera) {
    std::vector<std::unique_ptr<MeshRenderer>> meshRenderer{};
    for (auto &node: asset->nodes) {
        for (auto &mesh: node->meshes) {
            meshRenderer.push_back(std::unique_ptr<MeshRenderer>(
                    MeshRenderer::fromMesh(renderSystem, renderContext, shaderCache, node->modelMatrix, mesh, camera))
            );
        }
    }
    return new AssetRenderer(meshRenderer);
}


void AssetRenderer::render(LLGL::CommandBuffer &commandBuffer) {
    for (auto &renderer: renderers) {
        renderer->render(commandBuffer);
    }
}
