#pragma once

#include <LLGL/LLGL.h>
#include <Camera/Camera.hpp>

class MeshRenderer {

private:
    std::shared_ptr<LLGL::RenderSystem> &renderSystem;
    LLGL::ShaderProgram *shaderProgram;
    LLGL::Buffer *cameraShaderStateBuffer;
    LLGL::ResourceHeap *resourceHeap;
    LLGL::PipelineState *pipeline;
    const ICamera &camera;

    uint32_t numVertices;
    uint32_t numIndices;
    LLGL::Buffer *indexBuffer;
    std::vector<LLGL::Buffer *> buffers;
    LLGL::BufferArray *bufferArray;
    std::vector<LLGL::VertexFormat> vertexFormats;

public:
    virtual ~MeshRenderer();

    // TODO: Testing only
    static MeshRenderer *
    newTriangleMeshRenderer(std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                            LLGL::RenderContext *renderContext,
                            const ICamera &camera);

    void render(LLGL::CommandBuffer &commandBuffer);

private:
    MeshRenderer(std::shared_ptr<LLGL::RenderSystem> &renderSystem, LLGL::RenderContext *renderContext,
                 LLGL::ShaderProgram *shaderProgram,
                 uint32_t numIndices,
                 LLGL::Buffer *indexBuffer,
                 std::vector<LLGL::Buffer *> buffers,
                 LLGL::BufferArray *bufferArray,
                 const std::vector<LLGL::VertexFormat> &vertexFormats, uint32_t numVertices,
                 const ICamera &camera);

    void updateCameraShaderStateBuffer(LLGL::CommandBuffer &commandBuffer);
};