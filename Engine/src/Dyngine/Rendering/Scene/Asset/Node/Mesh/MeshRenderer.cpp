#include "Dyngine/Rendering/Scene/Asset/Node/Mesh/MeshRenderer.hpp"
#include "Dyngine/Rendering/Shader/ShaderUtil.hpp"
#include "ErrorHandling/IllegalStateException.hpp"
#include "ErrorHandling/IllegalArgumentException.hpp"
#include "LLGL/Utility.h"
#include "glm/glm.hpp"

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

void AddTextureBinding(LLGL::PipelineLayoutDescriptor &layoutDesc, uint32_t slot) {
    layoutDesc.bindings.push_back(
            LLGL::BindingDescriptor{LLGL::ResourceType::Sampler, 0,
                                    LLGL::StageFlags::FragmentStage, slot}
    );
    layoutDesc.bindings.push_back(
            LLGL::BindingDescriptor{LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled,
                                    LLGL::StageFlags::FragmentStage, slot}
    );
}

void AddTextureResourceIfExists(LLGL::ResourceHeapDescriptor &resourceHeapDescriptor, LLGL::Sampler *sampler,
                                LLGL::Texture *texture) {
    if (sampler == nullptr) {
        return;
    }
    if (sampler != nullptr && texture == nullptr) {
        RAISE_EXCEPTION(errorhandling::IllegalArgumentException,
                        "Texture must not be null, if a sampler is provided"
        );
    }
    resourceHeapDescriptor.resourceViews.push_back(sampler);
    resourceHeapDescriptor.resourceViews.push_back(texture);
}

MeshRenderer::MeshRenderer(const std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                           LLGL::RenderTarget *renderTarget,
                           const std::shared_ptr<ShaderCache> &shaderCache,
                           ShaderUsageHandle shaderUsageHandle,
                           const Node &parentNode,
                           const std::shared_ptr<Mesh> &mesh,
                           LLGL::Buffer *cameraShaderStateBuffer,
                           LLGL::Buffer *lightsShaderStateBuffer)
        : renderSystem(renderSystem),
          shaderCache(shaderCache),
          shaderUsageHandle(shaderUsageHandle),
          parentNode(parentNode),
          mesh(mesh) {

    // Create pipeline layout
    LLGL::PipelineLayoutDescriptor layoutDesc{};

    // 1. Camera shader state buffer
    layoutDesc.bindings.push_back(LLGL::BindingDescriptor{LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer,
                                                          LLGL::StageFlags::VertexStage |
                                                          LLGL::StageFlags::FragmentStage, 0u});

    // Add resource bindings for textures depending on whether they are present
    {
        auto material = mesh->material;

        // 1. Texture present flag integer
        layoutDesc.bindings.push_back(
                LLGL::BindingDescriptor{LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer,
                                        LLGL::StageFlags::FragmentStage, 1u});

        // 2. Albedo map
        if (material->albedoTexture != nullptr) {
            AddTextureBinding(layoutDesc, 2u);
        }
        // 3. Normal map
        if (material->normalTexture != nullptr) {
            AddTextureBinding(layoutDesc, 3u);
        }
        // 4. RMA map
        if (material->rmaTexture != nullptr) {
            AddTextureBinding(layoutDesc, 4u);
        }
    }

    // 4. Add lights shader state bindings
    layoutDesc.bindings.push_back(
            LLGL::BindingDescriptor{LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer,
                                    LLGL::StageFlags::FragmentStage, 4u}
    );

    // Setup pipeline layout
    LLGL::PipelineLayout *pipelineLayout = renderSystem->CreatePipelineLayout(layoutDesc);

    // Create pipeline
    {
        LLGL::GraphicsPipelineDescriptor pipelineDescriptor{
                .pipelineLayout = pipelineLayout,
                .shaderProgram = shaderUsageHandle.shaderProgram,
                .renderPass = renderTarget->GetRenderPass(),
                .primitiveTopology = GetPrimitiveTopology(mesh->meshRenderMode),
                .depth = {
                        .testEnabled = true,
                        .writeEnabled = true,
                        .compareOp = LLGL::CompareOp::Less
                },
                .rasterizer = {
                        .cullMode = LLGL::CullMode::Front,
                        .multiSampleEnabled = true,
                },
        };

        pipeline = renderSystem->CreatePipelineState(pipelineDescriptor);
    }

    // Create resource heap
    LLGL::ResourceHeapDescriptor resourceHeapDesc{
            .pipelineLayout = pipelineLayout
    };

    // 1. Camera shader state buffer
    resourceHeapDesc.resourceViews.push_back(cameraShaderStateBuffer);

    // Add samplers + textures
    {
        auto material = mesh->material;

        // 1. Texture present flag integer
        resourceHeapDesc.resourceViews.push_back(material->texturePresentFlagsBuffer);

        // 2. Albedo map
        AddTextureResourceIfExists(resourceHeapDesc, material->albedoSampler, material->albedoTexture);
        // 3. Normal map
        AddTextureResourceIfExists(resourceHeapDesc, material->normalSampler, material->normalTexture);
        // 4. RMA map
        AddTextureResourceIfExists(resourceHeapDesc, material->rmaSampler, material->rmaTexture);
    }

    // 4. Add lights shader state buffer
    resourceHeapDesc.resourceViews.push_back(lightsShaderStateBuffer);

    resourceHeap = renderSystem->CreateResourceHeap(resourceHeapDesc);
    renderSystem->Release(*pipelineLayout);
}

MeshRenderer *MeshRenderer::fromMesh(const std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                                     LLGL::RenderTarget *renderTarget,
                                     const std::shared_ptr<ShaderCache> &shaderCache,
                                     const Node &parentNode,
                                     const std::shared_ptr<Mesh> &mesh,
                                     LLGL::Buffer *cameraStateBuffer,
                                     LLGL::Buffer *lightShaderStateBuffer
) {
    std::vector<LLGL::VertexAttribute> vertexInputAttributes{};
    for (const auto &vertexFormat: mesh->vertexFormats) {
        vertexInputAttributes.insert(vertexInputAttributes.end(), vertexFormat.attributes.begin(),
                                     vertexFormat.attributes.end());
    }
    ShaderUsageHandle shaderUsageHandle = shaderCache->getOrCompile(
            {
//                    .resourcePath = mesh->material->albedoTexture != nullptr ? "/shaders/textured.dsp" : "/shaders/untextured.dsp", // TODO: remove hack
                    .resourcePath = "/shaders/pbr.dsp",
                    .vertexInputAttributes=vertexInputAttributes,
                    .fragmentOutputAttributes={}
            }
    );
    return new MeshRenderer(renderSystem, renderTarget, shaderCache,
                            shaderUsageHandle,
                            parentNode,
                            mesh,
                            cameraStateBuffer,
                            lightShaderStateBuffer
    );
}

void MeshRenderer::render(LLGL::CommandBuffer &commandBuffer) {
    commandBuffer.SetPipelineState(*pipeline);
    commandBuffer.SetResourceHeap(*resourceHeap);

    commandBuffer.SetVertexBufferArray(*mesh->bufferArray);

    if (mesh->indexBuffer != nullptr) {
        commandBuffer.SetIndexBuffer(*mesh->indexBuffer);
        commandBuffer.DrawIndexed(mesh->numIndices, 0, 0);
    } else {
        commandBuffer.Draw(mesh->numVertices, 0);
    }
}

MeshRenderer::~MeshRenderer() {
    renderSystem->Release(*resourceHeap);
    renderSystem->Release(*pipeline);
}