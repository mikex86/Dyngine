#include "Rendering/Scene/SceneRenderer.hpp"

SceneRenderer::SceneRenderer(const std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                             LLGL::RenderContext *renderContext, ShaderCache &shaderCache, Scene &scene)
        : renderSystem(renderSystem), renderContext(renderContext), shaderCache(shaderCache), scene(scene) {
}

void SceneRenderer::render(LLGL::CommandBuffer &commandBuffer) {
    addNewAssetRenders();
    updateLightData();

    for (auto &renderer: renderers) {
        renderer->render(commandBuffer);
    }
}

void SceneRenderer::updateLightData() {

}

void SceneRenderer::addNewAssetRenders() {
    auto &newAssets = scene.getNewAssets();
    while (!newAssets.empty()) {
        auto asset = newAssets.front();
        newAssets.pop();
        auto renderer = AssetRenderer::fromAsset(renderSystem, renderContext, shaderCache, asset, scene.getCamera());
        renderers.push_back(std::unique_ptr<AssetRenderer>(renderer));
    }
}