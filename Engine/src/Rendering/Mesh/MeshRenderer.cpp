#include "Rendering/Mesh/MeshRenderer.hpp"
#include "Rendering/Shader/ShaderUtil.hpp"
#include <ErrorHandling/IllegalStateException.hpp>
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
          camera(camera) {

    // Create pipeline layout
    LLGL::PipelineLayoutDescriptor layoutDesc{
            .bindings = {
                    // 1. Camera shader state buffer
                    LLGL::BindingDescriptor{LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer,
                                            LLGL::StageFlags::VertexStage, 0u},
            }
    };
    // Add resource bindings for textures depending on whether they are present
    {
        auto material = mesh->material;
        if (material->albedoTexture != nullptr) {
            layoutDesc.bindings.push_back(
                    // 3. Albedo texture sampler
                    LLGL::BindingDescriptor{LLGL::ResourceType::Sampler, 0,
                                            LLGL::StageFlags::FragmentStage, 0u}
            );
            layoutDesc.bindings.push_back(
                    // 4. Albedo texture
                    LLGL::BindingDescriptor{LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled,
                                            LLGL::StageFlags::FragmentStage, 1u}
            );
        }
    }

    // Setup pipeline layout
    LLGL::PipelineLayout *pipelineLayout = renderSystem->CreatePipelineLayout(layoutDesc);

    // Create pipeline
    {
        LLGL::GraphicsPipelineDescriptor pipelineDescriptor{
                .pipelineLayout = pipelineLayout,
                .shaderProgram = shaderUsageHandle.shaderProgram,
                .renderPass = renderContext->GetRenderPass(),
                .primitiveTopology = GetPrimitiveTopology(mesh->meshRenderMode),
                .depth = {
                        .testEnabled = true,
                        .writeEnabled = true,
                        .compareOp = LLGL::CompareOp::Less
                },
                .rasterizer = {
                        .cullMode = LLGL::CullMode::Front,
                }
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
    // Add samplers + textures
    {
        auto material = mesh->material;

        bool hasAlbedo = material->albedoTexture != nullptr;
        if (hasAlbedo) {
            if (material->albedoSampler == nullptr) {
                RAISE_EXCEPTION(errorhandling::IllegalStateException,
                                "Albedo sampler not present despite the albedo texture being marked as present in material."
                );
            }
            resourceHeapDesc.resourceViews.push_back(material->albedoSampler);
        }
        if (hasAlbedo) {
            assert(material->albedoTexture != nullptr);
            resourceHeapDesc.resourceViews.push_back(material->albedoTexture);
        }
    }
    resourceHeap = renderSystem->CreateResourceHeap(resourceHeapDesc);
    renderSystem->Release(*pipelineLayout);


    this->mesh = std::move(mesh);
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
                    .resourcePath = mesh->material->albedoTexture != nullptr ? "/shaders/textured.dsp" : "/shaders/untextured.dsp", // TODO: remove hack
//                    .resourcePath = "/shaders/untextured.dsp",
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