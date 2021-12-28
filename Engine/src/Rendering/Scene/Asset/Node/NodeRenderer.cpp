#include "Rendering/Scene/Asset/Node/NodeRenderer.hpp"
#include "LLGL/Utility.h"

NodeRenderer *
NodeRenderer::fromNode(const std::shared_ptr<LLGL::RenderSystem> &renderSystem, LLGL::RenderContext *renderContext,
                       ShaderCache &shaderCache, const std::shared_ptr<Asset> &asset, ICamera &camera,
                       const std::unique_ptr<Node> &node) {
    auto meshRenderers = std::vector<std::unique_ptr<MeshRenderer>>();
    // Create shader state buffer
    auto cameraShaderStateBuffer = renderSystem->CreateBuffer(
            LLGL::ConstantBufferDesc(
                    sizeof(CameraShaderState)
            ),
            &camera.getCameraShaderState()
    );
    for (auto &mesh: node->getMeshes()) {
        meshRenderers.push_back(std::unique_ptr<MeshRenderer>(
                MeshRenderer::fromMesh(renderSystem, renderContext, shaderCache, *node, mesh, cameraShaderStateBuffer))
        );
    }
    return new NodeRenderer(renderSystem, camera, cameraShaderStateBuffer, meshRenderers);
}


NodeRenderer::NodeRenderer(const std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                           ICamera &camera,
                           LLGL::Buffer *cameraShaderStateBuffer,
                           std::vector<std::unique_ptr<MeshRenderer>> &meshRenderers) :
        renderSystem(renderSystem),
        camera(camera),
        cameraShaderStateBuffer(cameraShaderStateBuffer),
        meshRenderers(std::move(meshRenderers)) {
}

void NodeRenderer::render(LLGL::CommandBuffer &commandBuffer) {
    updateCameraShaderStateBuffer(commandBuffer);
    for (auto &mesh: meshRenderers) {
        mesh->render(commandBuffer);
    }
}

void NodeRenderer::updateCameraShaderStateBuffer(LLGL::CommandBuffer &commandBuffer) {
    commandBuffer.UpdateBuffer(*cameraShaderStateBuffer, 0, &camera.getCameraShaderState(), sizeof(CameraShaderState));
}

NodeRenderer::~NodeRenderer() {
    renderSystem->Release(*cameraShaderStateBuffer);
}

