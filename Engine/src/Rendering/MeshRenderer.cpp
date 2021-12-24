#include <Rendering/MeshRenderer.hpp>
#include <glm/glm.hpp>
#include <Shader/ShaderUtil.hpp>
#include <LLGL/Utility.h>

MeshRenderer::MeshRenderer(std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                           LLGL::RenderContext *renderContext,
                           LLGL::ShaderProgram *shaderProgram,
                           uint32_t numIndices,
                           LLGL::Buffer *indexBuffer,
                           std::vector<LLGL::Buffer *> buffers,
                           LLGL::BufferArray *bufferArray,
                           const std::vector<LLGL::VertexFormat> &vertexFormats,
                           uint32_t numVertices,
                           const ICamera &camera)
        : renderSystem(renderSystem), shaderProgram(shaderProgram), numIndices(numIndices), indexBuffer(indexBuffer),
          buffers(std::move(buffers)), bufferArray(bufferArray), vertexFormats(vertexFormats),
          numVertices(numVertices), camera(camera) {

    // Setup pipeline layout
    LLGL::PipelineLayout *pipelineLayout = renderSystem->CreatePipelineLayout(
            // camera shader state buffer
            LLGL::PipelineLayoutDesc("cbuffer(CameraShaderState@1):frag")
    );

    // Create pipeline
    {
        LLGL::GraphicsPipelineDescriptor pipelineDescriptor{
                .pipelineLayout = pipelineLayout,
                .shaderProgram = shaderProgram,
                .renderPass = renderContext->GetRenderPass(),
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


MeshRenderer *
MeshRenderer::newTriangleMeshRenderer(std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                                      LLGL::RenderContext *renderContext,
                                      const ICamera &camera) {
    std::vector<LLGL::Buffer *> buffers{};
    std::vector<LLGL::VertexFormat> vertexFormats{};

    LLGL::Buffer *indexBuffer;
    // indices
    {
        uint32_t indices[] = {
                0, 1, 2
        };
        LLGL::BufferDescriptor bufferDesc{
                .size = sizeof(indices),
                .format = LLGL::Format::R32UInt,
                .bindFlags = LLGL::BindFlags::IndexBuffer
        };
        indexBuffer = renderSystem->CreateBuffer(bufferDesc, &indices);
    }
    // positions
    {
        LLGL::VertexFormat vertexFormat{};
        vertexFormat.AppendAttribute({"position", LLGL::Format::RGB32Float, 0});
        vertexFormat.SetSlot(0);
        const float s = 1.0f;
        glm::vec3 positions[] = {
                {0,  s,  0},
                {s,  -s, 0},
                {-s, -s, 0}
        };
        LLGL::BufferDescriptor bufferDesc{
                .size = sizeof(positions),
                .format = LLGL::Format::RGB32Float,
                .bindFlags = LLGL::BindFlags::VertexBuffer,
                .vertexAttribs = vertexFormat.attributes
        };
        LLGL::Buffer *positionBuffer = renderSystem->CreateBuffer(bufferDesc, &positions);
        buffers.push_back(positionBuffer);
        vertexFormats.push_back(vertexFormat);
    }
    // colors
    {
        LLGL::VertexFormat vertexFormat{};
        vertexFormat.AppendAttribute({"color", LLGL::Format::RGBA32Float, 1});
        vertexFormat.SetSlot(1);
        glm::vec4 colors[] = {
                {1.0f, 0.0f, 0.0f, 1.0f},
                {0.0f, 1.0f, 0.0f, 1.0f},
                {0.0f, 0.0f, 1.0f, 1.0f}
        };
        LLGL::BufferDescriptor bufferDesc{
                .size = sizeof(colors),
                .format = LLGL::Format::RGBA32Float,
                .bindFlags = LLGL::BindFlags::VertexBuffer,
                .vertexAttribs = vertexFormat.attributes
        };
        LLGL::Buffer *colorBuffer = renderSystem->CreateBuffer(bufferDesc, &colors);
        buffers.push_back(colorBuffer);
        vertexFormats.push_back(vertexFormat);
    }

    std::vector<LLGL::VertexAttribute> vertexInputAttributes{};
    for (const auto &vertexFormat: vertexFormats) {
        vertexInputAttributes.insert(vertexInputAttributes.end(), vertexFormat.attributes.begin(),
                                     vertexFormat.attributes.end());
    }

    LLGL::BufferArray *bufferArray = renderSystem->CreateBufferArray(buffers.size(), buffers.data());
    Dpac::ReadOnlyArchive engineResources = Dpac::ReadOnlyArchive::Open("EngineResources.dpac");

    // Setup ShaderProgram
    LLGL::ShaderProgram *shaderProgram = ShaderUtil::LoadDShaderPackage(
            engineResources.getEntryStream("/shaders/triangle.dsp"),
            *renderSystem,
            vertexInputAttributes,
            {}
    );
    return new MeshRenderer(renderSystem, renderContext, shaderProgram,
                            3, indexBuffer, buffers, bufferArray, vertexFormats, 3, camera
    );
}


void MeshRenderer::render(LLGL::CommandBuffer &commandBuffer) {
    commandBuffer.SetPipelineState(*pipeline);
    commandBuffer.SetResourceHeap(*resourceHeap);
    updateCameraShaderStateBuffer(commandBuffer);

    commandBuffer.SetIndexBuffer(*indexBuffer);
    commandBuffer.SetVertexBufferArray(*bufferArray);
    commandBuffer.DrawIndexed(numIndices, 0, 0);
}

void MeshRenderer::updateCameraShaderStateBuffer(LLGL::CommandBuffer &commandBuffer) {
    commandBuffer.UpdateBuffer(*cameraShaderStateBuffer, 0, &camera.getCameraShaderState(), sizeof(CameraShaderState));
}

MeshRenderer::~MeshRenderer() {
    renderSystem->Release(*indexBuffer);
    for (auto &buffer: buffers) {
        renderSystem->Release(*buffer);
    }
    renderSystem->Release(*bufferArray);
    renderSystem->Release(*shaderProgram);
    renderSystem->Release(*cameraShaderStateBuffer);
    renderSystem->Release(*resourceHeap);
    renderSystem->Release(*pipeline);
}