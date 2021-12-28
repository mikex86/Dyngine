#pragma once

#include "LLGL/LLGL.h"
#include "Mesh.hpp"
#include "Rendering/Scene/Camera/Camera.hpp"
#include "Rendering/Shader/ShaderCache.hpp"
#include "Rendering/Scene/Asset/AssetLoader.hpp"

class MeshRenderer {

private:
    std::shared_ptr<LLGL::RenderSystem> renderSystem;
    ShaderCache &shaderCache;
    ShaderUsageHandle shaderUsageHandle;
    LLGL::ResourceHeap *resourceHeap;
    LLGL::PipelineState *pipeline;
    const Node &parentNode;
    std::shared_ptr<Mesh> mesh;

public:
    MeshRenderer(const std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                 LLGL::RenderContext *renderContext,
                 ShaderCache &shaderCache,
                 ShaderUsageHandle shaderUsageHandle,
                 const Node &parentNode,
                 const std::shared_ptr<Mesh> &mesh,
                 LLGL::Buffer *cameraStateBuffer);

    virtual ~MeshRenderer();

    static MeshRenderer *fromMesh(const std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                                  LLGL::RenderContext *renderContext,
                                  ShaderCache &shaderCache,
                                  const Node &parentNode,
                                  const std::shared_ptr<Mesh> &mesh,
                                  LLGL::Buffer *cameraStateBuffer
    );

    void render(LLGL::CommandBuffer &commandBuffer);

};