#pragma once

#include <LLGL/CommandBuffer.h>
#include "Rendering/Scene/Asset/Node/Node.hpp"
#include "Rendering/Scene/Asset/Node/Mesh/MeshRenderer.hpp"
#include "Rendering/Scene/Camera/Camera.hpp"

class NodeRenderer {

private:
    ICamera &camera;
    LLGL::Buffer *cameraShaderStateBuffer;
    std::shared_ptr<LLGL::RenderSystem> renderSystem;
    std::vector<std::unique_ptr<MeshRenderer>> meshRenderers;

public:
    NodeRenderer(const std::shared_ptr<LLGL::RenderSystem> &renderSystem, ICamera &camera,
                 LLGL::Buffer *cameraShaderStateBuffer, std::vector<std::unique_ptr<MeshRenderer>> &meshRenderers);

    void render(LLGL::CommandBuffer &commandBuffer);

    static NodeRenderer *fromNode(const std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                                  LLGL::RenderContext *renderContext,
                                  ShaderCache &shaderCache,
                                  const std::shared_ptr<Asset> &asset,
                                  ICamera &camera,
                                  const std::unique_ptr<Node> &node
    );

    virtual ~NodeRenderer();

private:
    void updateCameraShaderStateBuffer(LLGL::CommandBuffer &commandBuffer);

};