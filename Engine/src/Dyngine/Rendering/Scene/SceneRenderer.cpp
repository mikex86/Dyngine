#include "Dyngine/Rendering/Scene/SceneRenderer.hpp"

SceneRenderer::SceneRenderer(const std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                             LLGL::RenderTarget *renderTarget, const std::shared_ptr<ShaderCache> &shaderCache, std::unique_ptr<Scene> scene)
        : renderSystem(renderSystem), renderTarget(renderTarget), shaderCache(shaderCache), scene(std::move(scene)) {
    LLGL::BufferDescriptor bufferDescriptor = {
            .size = sizeof(LightsShaderState),
            .bindFlags = LLGL::BindFlags::ConstantBuffer
    };
    lightsShaderStateBuffer = renderSystem->CreateBuffer(bufferDescriptor, &lightsShaderState);
}

void SceneRenderer::render(LLGL::CommandBuffer &commandBuffer) {
    addNewAssetRenders();
    updateLightData(commandBuffer);

    for (auto &renderer: renderers) {
        renderer->render(commandBuffer);
    }
}

void SceneRenderer::updateLightData(LLGL::CommandBuffer &commandBuffer) {
    if (!scene->haveLightsChanged()) {
        return;
    }
    auto &lights = scene->getLights();

    lightsShaderState.numPointLights = 0;
    for (const auto &entry: lights) {
        auto light = entry.second;
        if (light->getLightType() == LightType::POINT) {
            auto index = lightsShaderState.numPointLights++;
            lightsShaderState.pointLightPositions[index] = {light->getPosition(), 0}; // w = alignment dummy
            lightsShaderState.pointLightColors[index] = {light->getColor(), light->getIntensity()};
        }
    }
    commandBuffer.UpdateBuffer(*lightsShaderStateBuffer, 0, &lightsShaderState,sizeof(LightsShaderState));
}

void SceneRenderer::addNewAssetRenders() {
    // TODO: This system is stupid
    auto &newAssets = scene->getNewAssets();
    while (!newAssets.empty()) {
        auto asset = newAssets.front();
        newAssets.pop();
        auto renderer = AssetRenderer::fromAsset(renderSystem, renderTarget, shaderCache, asset, scene->getCamera(),
                                                 lightsShaderStateBuffer);
        renderers.push_back(std::unique_ptr<AssetRenderer>(renderer));
    }
}