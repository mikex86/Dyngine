#include <Mesh/MeshRenderer.hpp>
#include <Shader/ShaderUtil.hpp>
#include <ErrorHandling/IllegalArgumentException.hpp>
#include <LLGL/Utility.h>
#include <glm/glm.hpp>

LLGL::PrimitiveTopology GetPrimitiveTopology(MeshRenderMode mode) {
    switch (mode) {
        case MeshRenderMode::TRIANGLES:
            return LLGL::PrimitiveTopology::TriangleList;
        case MeshRenderMode::TRIANGLE_FAN:
            return LLGL::PrimitiveTopology::TriangleFan;
        case MeshRenderMode::TRIANGLE_STRIP:
            return LLGL::PrimitiveTopology::TriangleStrip;
        case MeshRenderMode::LINES:
            return LLGL::PrimitiveTopology::LineList;
        case MeshRenderMode::LINE_STRIP:
            return LLGL::PrimitiveTopology::LineStrip;
        case MeshRenderMode::LINE_LOOP:
            return LLGL::PrimitiveTopology::LineLoop;
        case MeshRenderMode::POINTS:
            return LLGL::PrimitiveTopology::PointList;
        default:
            RAISE_EXCEPTION(errorhandling::IllegalArgumentException,
                            "Unknown mesh render mode: " + std::to_string(static_cast<int>(mode))
            );
    }
}

MeshRenderer::MeshRenderer(std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                           LLGL::RenderContext *renderContext,
                           ShaderCache &shaderCache,
                           ShaderUsageHandle shaderUsageHandle,
                           glm::mat4 &modelMatrix,
                           std::unique_ptr<Mesh> &mesh,
                           ICamera &camera)
        : renderSystem(renderSystem), shaderCache(shaderCache),
          shaderUsageHandle(shaderUsageHandle),
          modelMatrix(modelMatrix),
          mesh(std::move(mesh)),
          camera(camera) {

    // Setup pipeline layout
    LLGL::PipelineLayout *pipelineLayout = renderSystem->CreatePipelineLayout(
            // camera shader state buffer
            LLGL::PipelineLayoutDesc("cbuffer(CameraShaderState@1):frag")
    );

    // Create pipeline
    {
        LLGL::GraphicsPipelineDescriptor pipelineDescriptor{
                .pipelineLayout = pipelineLayout,
                .shaderProgram = shaderUsageHandle.shaderProgram,
                .renderPass = renderContext->GetRenderPass(),
                .primitiveTopology = GetPrimitiveTopology(this->mesh->meshRenderMode)
        };

        pipeline = renderSystem->CreatePipelineState(pipelineDescriptor);
    }

    // Create shader state buffer
    cameraShaderStateBuffer = renderSystem->CreateBuffer(
            LLGL::ConstantBufferDesc(
                    sizeof(CameraShaderState)
            ),
            &camera.getCameraShaderState()
    );

    // Create resource heap
    LLGL::ResourceHeapDescriptor resourceHeapDesc{
            .pipelineLayout = pipelineLayout,
            .resourceViews = {cameraShaderStateBuffer}
    };
    resourceHeap = renderSystem->CreateResourceHeap(resourceHeapDesc);
    renderSystem->Release(*pipelineLayout);
}

MeshRenderer *MeshRenderer::fromMesh(std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                                     LLGL::RenderContext *renderContext,
                                     ShaderCache &shaderCache,
                                     glm::mat4 &modelMatrix,
                                     std::unique_ptr<Mesh> &mesh,
                                     ICamera &camera
) {
    std::vector<LLGL::VertexAttribute> vertexInputAttributes{};
    for (const auto &vertexFormat: mesh->vertexFormats) {
        vertexInputAttributes.insert(vertexInputAttributes.end(), vertexFormat.attributes.begin(),
                                     vertexFormat.attributes.end());
    }
    ShaderUsageHandle shaderUsageHandle = shaderCache.getOrCompile(
            {
                    .resourcePath = "/shaders/triangle.dsp", // TODO: use resource path
                    .vertexInputAttributes=vertexInputAttributes,
                    .fragmentOutputAttributes={}
            }
    );
    return new MeshRenderer(renderSystem, renderContext, shaderCache,
                            shaderUsageHandle,
                            modelMatrix,
                            mesh,
                            camera
    );
}

void MeshRenderer::render(LLGL::CommandBuffer &commandBuffer) {
    commandBuffer.SetPipelineState(*pipeline);
    commandBuffer.SetResourceHeap(*resourceHeap);

    camera.setModelMatrix(modelMatrix);
    updateCameraShaderStateBuffer(commandBuffer);

    commandBuffer.SetVertexBufferArray(*mesh->bufferArray);

    if (mesh->indexBuffer != nullptr) {
        commandBuffer.SetIndexBuffer(*mesh->indexBuffer);
        commandBuffer.DrawIndexed(mesh->numIndices, 0, 0);
    } else {
        commandBuffer.Draw(mesh->numVertices, 0);
    }
}

void MeshRenderer::updateCameraShaderStateBuffer(LLGL::CommandBuffer &commandBuffer) {
    commandBuffer.UpdateBuffer(*cameraShaderStateBuffer, 0, &camera.getCameraShaderState(), sizeof(CameraShaderState));
}

MeshRenderer::~MeshRenderer() {
    renderSystem->Release(*cameraShaderStateBuffer);
    renderSystem->Release(*resourceHeap);
    renderSystem->Release(*pipeline);
}