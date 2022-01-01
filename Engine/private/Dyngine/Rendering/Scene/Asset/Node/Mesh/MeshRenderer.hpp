#pragma once

#include <LLGL/LLGL.h>
#include "Dyngine/Rendering/Scene/Asset/Node/Mesh/Mesh.hpp"
#include "Dyngine/Rendering/Scene/Camera/Camera.hpp"
#include "Dyngine/Rendering/Shader/ShaderCache.hpp"
#include "Dyngine/Rendering/Scene/Asset/AssetLoader.hpp"

class MeshRenderer {

private:
    std::shared_ptr<LLGL::RenderSystem> renderSystem;
    std::shared_ptr<ShaderCache> shaderCache;
    ShaderUsageHandle shaderUsageHandle;
    LLGL::ResourceHeap *resourceHeap;
    LLGL::PipelineState *pipeline;
    const Node &parentNode;
    std::shared_ptr<Mesh> mesh;

public:
    MeshRenderer(const std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                 LLGL::RenderTarget *renderTarget,
                 const std::shared_ptr<ShaderCache> &shaderCache,
                 ShaderUsageHandle shaderUsageHandle,
                 const Node &parentNode,
                 const std::shared_ptr<Mesh> &mesh,
                 LLGL::Buffer *cameraStateBuffer,
                 LLGL::Buffer *lightShaderStateBuffer);

    virtual ~MeshRenderer();

    static MeshRenderer *fromMesh(const std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                                  LLGL::RenderTarget *renderTarget,
                                  const std::shared_ptr<ShaderCache> &shaderCache,
                                  const Node &parentNode,
                                  const std::shared_ptr<Mesh> &mesh,
                                  LLGL::Buffer *cameraStateBuffer,
                                  LLGL::Buffer *lightShaderStateBuffer
    );

    void render(LLGL::CommandBuffer &commandBuffer);

};