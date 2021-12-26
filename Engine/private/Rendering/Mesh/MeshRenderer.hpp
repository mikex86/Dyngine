#pragma once

#include <LLGL/LLGL.h>
#include "Mesh.hpp"
#include "Rendering/Camera/Camera.hpp"
#include "Rendering/Shader/ShaderCache.hpp"
#include "Rendering/Asset/AssetLoader.hpp"

class MeshRenderer {

private:
    std::shared_ptr<LLGL::RenderSystem> &renderSystem;
    ShaderCache &shaderCache;
    ShaderUsageHandle shaderUsageHandle;
    LLGL::Buffer *cameraShaderStateBuffer;
    LLGL::ResourceHeap *resourceHeap;
    LLGL::PipelineState *pipeline;
    ICamera &camera;
    glm::mat4 modelMatrix;
    std::unique_ptr<Mesh> mesh;

public:
    MeshRenderer(std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                 LLGL::RenderContext *renderContext,
                 ShaderCache &shaderCache,
                 ShaderUsageHandle shaderUsageHandle,
                 glm::mat4 &modelMatrix,
                 std::unique_ptr<Mesh> &mesh,
                 ICamera &camera);

    virtual ~MeshRenderer();

    static MeshRenderer *fromMesh(std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                                  LLGL::RenderContext *renderContext,
                                  ShaderCache &shaderCache,
                                  glm::mat4 &modelMatrix,
                                  std::unique_ptr<Mesh> &mesh,
                                  ICamera &camera
    );

    void render(LLGL::CommandBuffer &commandBuffer);

private:

    void updateCameraShaderStateBuffer(LLGL::CommandBuffer &commandBuffer);
};