#include "Dyngine/Rendering/Scene/Asset/AssetLoader.hpp"

#include "DAsset/Asset.hpp"
#include "ErrorHandling/IllegalArgumentException.hpp"
#include "ErrorHandling/IllegalStateException.hpp"

#include "glm/gtx/quaternion.hpp"
#include <optional>
#include <queue>
#include "stb_image.h"

LLGL::Format GetBufferFormat(DAsset::DataType dataType, DAsset::ComponentType componentType) {
    switch (componentType) {
        case DAsset::ComponentType::SCALAR: {
            switch (dataType) {
                case DAsset::DataType::UNSIGNED_BYTE:
                    return LLGL::Format::R8UInt;
                case DAsset::DataType::BYTE:
                    return LLGL::Format::R8SInt;
                case DAsset::DataType::UNSIGNED_SHORT:
                    return LLGL::Format::R16UInt;
                case DAsset::DataType::SHORT:
                    return LLGL::Format::R16SInt;
                case DAsset::DataType::UNSIGNED_INT:
                    return LLGL::Format::R32UInt;
                case DAsset::DataType::INT:
                    return LLGL::Format::R32SInt;
                case DAsset::DataType::FLOAT:
                    return LLGL::Format::R32Float;
                case DAsset::DataType::DOUBLE:
                    return LLGL::Format::R64Float;
            }
        }
        case DAsset::ComponentType::VEC2: {
            switch (dataType) {
                case DAsset::DataType::UNSIGNED_BYTE:
                    return LLGL::Format::RG8UInt;
                case DAsset::DataType::BYTE:
                    return LLGL::Format::RG8SInt;
                case DAsset::DataType::UNSIGNED_SHORT:
                    return LLGL::Format::RG16UInt;
                case DAsset::DataType::SHORT:
                    return LLGL::Format::RG16SInt;
                case DAsset::DataType::UNSIGNED_INT:
                    return LLGL::Format::RG32UInt;
                case DAsset::DataType::INT:
                    return LLGL::Format::RG32SInt;
                case DAsset::DataType::FLOAT:
                    return LLGL::Format::RG32Float;
                case DAsset::DataType::DOUBLE:
                    return LLGL::Format::RG64Float;
            }
        }
        case DAsset::ComponentType::VEC3: {
            switch (dataType) {
                case DAsset::DataType::UNSIGNED_BYTE:
                    return LLGL::Format::RGB8UInt;
                case DAsset::DataType::BYTE:
                    return LLGL::Format::RGB8SInt;
                case DAsset::DataType::UNSIGNED_SHORT:
                    return LLGL::Format::RGB16UInt;
                case DAsset::DataType::SHORT:
                    return LLGL::Format::RGB16SInt;
                case DAsset::DataType::UNSIGNED_INT:
                    return LLGL::Format::RGB32UInt;
                case DAsset::DataType::INT:
                    return LLGL::Format::RGB32SInt;
                case DAsset::DataType::FLOAT:
                    return LLGL::Format::RGB32Float;
                case DAsset::DataType::DOUBLE:
                    return LLGL::Format::RGB64Float;
            }
        }
        case DAsset::ComponentType::VEC4: {
            switch (dataType) {
                case DAsset::DataType::UNSIGNED_BYTE:
                    return LLGL::Format::RGBA8UInt;
                case DAsset::DataType::BYTE:
                    return LLGL::Format::RGBA8SInt;
                case DAsset::DataType::UNSIGNED_SHORT:
                    return LLGL::Format::RGBA16UInt;
                case DAsset::DataType::SHORT:
                    return LLGL::Format::RGBA16SInt;
                case DAsset::DataType::UNSIGNED_INT:
                    return LLGL::Format::RGBA32UInt;
                case DAsset::DataType::INT:
                    return LLGL::Format::RGBA32SInt;
                case DAsset::DataType::FLOAT:
                    return LLGL::Format::RGBA32Float;
                case DAsset::DataType::DOUBLE:
                    return LLGL::Format::RGBA64Float;
            }
        }
        default:
            break;
    }
    RAISE_EXCEPTION(errorhandling::IllegalArgumentException,
                    "Cannot retrieve Buffer Format for illegal data type - component type combination: " +
                    DAsset::GetDataTypeName(dataType) + ", " +
                    DAsset::GetComponentTypeName(componentType));
}


MeshRenderMode GetMeshRenderMode(DAsset::RenderMode dAssetRenderMode) {
    switch (dAssetRenderMode) {
        case DAsset::RenderMode::TRIANGLES:
            return MeshRenderMode::TRIANGLES;
        case DAsset::RenderMode::TRIANGLE_STRIP:
            return MeshRenderMode::TRIANGLE_STRIP;
        case DAsset::RenderMode::TRIANGLE_FAN:
            return MeshRenderMode::TRIANGLE_FAN;
        case DAsset::RenderMode::LINES:
            return MeshRenderMode::LINES;
        case DAsset::RenderMode::LINE_STRIP:
            return MeshRenderMode::LINE_STRIP;
        case DAsset::RenderMode::LINE_LOOP:
            return MeshRenderMode::LINE_LOOP;
        case DAsset::RenderMode::POINTS:
            return MeshRenderMode::POINTS;
        default:
            RAISE_EXCEPTION(errorhandling::IllegalArgumentException,
                            "Cannot retrieve MeshRenderMode for illegal render mode: " +
                            DAsset::GetRenderModeName(dAssetRenderMode)
            );
    }
}

uint32_t GetLocationIndex(DAsset::AttributeType type) {
    switch (type) {
        case DAsset::AttributeType::POSITION:
            return 0;
        case DAsset::AttributeType::NORMAL:
            return 1;
        case DAsset::AttributeType::TANGENT:
            return 2;
        case DAsset::AttributeType::TEX_COORD:
            return 3;
        case DAsset::AttributeType::COLOR:
            return 4;
        default:
            return -1;
    }
}

LLGL::Texture *LoadOptionalTexture(const std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                                   const std::optional<std::shared_ptr<DAsset::Texture>> &optionalTexture) {
    if (!optionalTexture.has_value()) {
        return nullptr;
    }
    auto texture = optionalTexture.value();
    auto textureBufferView = texture->bufferView;
    auto textureBuffer = textureBufferView.buffer;
    auto textureBufferData = textureBuffer->data;
    int32_t width{}, height{}, channels{};
    auto imageData = stbi_load_from_memory(&textureBufferData[textureBufferView.byteOffset],
                                           textureBufferView.byteLength, &width, &height, &channels, 4);
    auto imageDataSize = width * height * 4 * sizeof(uint8_t);
    if (!imageData) {
        RAISE_EXCEPTION(errorhandling::IllegalArgumentException,
                        "Cannot load texture from buffer view: " +
                        std::string(stbi_failure_reason())
        );
    }
    if (texture->width != width || texture->height != height) {
        RAISE_EXCEPTION(errorhandling::IllegalArgumentException,
                        "Cannot load texture from buffer view:"
                        "Texture dimensions described in texture data do not match meta data"
                        "MetadataDimensions: (width: " + std::to_string(texture->width) + ", height:" +
                        std::to_string(texture->height) + ")"
                                                          "TextureDataDimensions: (width:" + std::to_string(width) +
                        ", height:" + std::to_string(height) + ")"
        );
    }
    if (texture->channels != channels) {
        RAISE_EXCEPTION(errorhandling::IllegalArgumentException,
                        "Cannot load texture from buffer view:"
                        "Texture channels described in texture data do not match meta data"
                        "MetadataChannels: " + std::to_string(texture->channels) +
                        "TextureDataChannels: " + std::to_string(channels)
        );
    }
    if (width <= 0 || height <= 0) {
        RAISE_EXCEPTION(errorhandling::IllegalArgumentException,
                        "Cannot load texture from buffer view: "
                        "Metadata texture dimensions are invalid"
        );
    }

    LLGL::SrcImageDescriptor imageDescriptor{
            LLGL::ImageFormat::RGBA,
            LLGL::DataType::UInt8,
            imageData,
            imageDataSize
    };
    LLGL::TextureDescriptor textureDescriptor{
            .type = LLGL::TextureType::Texture2D,
            .miscFlags = LLGL::MiscFlags::GenerateMips,
            .format = LLGL::Format::RGBA8UNorm,
            .extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
            .mipLevels = 0
    };
    LLGL::Texture *llglTexture = renderSystem->CreateTexture(textureDescriptor, &imageDescriptor);
    delete[] imageData;

    return llglTexture;
}

LLGL::SamplerAddressMode GetLLGLAddressMode(DAsset::SamplerAddressMode addressMode) {
    switch (addressMode) {
        case DAsset::SamplerAddressMode::REPEAT:
            return LLGL::SamplerAddressMode::Repeat;
        case DAsset::SamplerAddressMode::MIRROR:
            return LLGL::SamplerAddressMode::Mirror;
        case DAsset::SamplerAddressMode::CLAMP:
            return LLGL::SamplerAddressMode::Clamp;
        case DAsset::SamplerAddressMode::BORDER:
            return LLGL::SamplerAddressMode::Border;
        case DAsset::SamplerAddressMode::MIRROR_ONCE:
            return LLGL::SamplerAddressMode::MirrorOnce;
        default:
            RAISE_EXCEPTION(errorhandling::IllegalArgumentException,
                            "Cannot retrieve TextureAddressMode for illegal address mode: " +
                            DAsset::GetTextureAddressModeName(addressMode)
            );
    }
}

LLGL::SamplerFilter GetLLGLFilter(DAsset::SamplerFilter filter) {
    switch (filter) {
        case DAsset::SamplerFilter::NEAREST:
            return LLGL::SamplerFilter::Nearest;
        case DAsset::SamplerFilter::LINEAR:
            return LLGL::SamplerFilter::Linear;
        default:
            RAISE_EXCEPTION(errorhandling::IllegalArgumentException,
                            "Cannot retrieve TextureFilter for illegal filter: " +
                            DAsset::GetTextureFilterName(filter)
            );
    }
}

LLGL::Sampler *CreateOptionalSampler(const std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                                     const std::optional<std::shared_ptr<DAsset::Texture>> &optionalTexture) {
    if (!optionalTexture.has_value()) {
        return nullptr;
    }
    auto texture = optionalTexture.value();
    auto llglAddressModeU = GetLLGLAddressMode(texture->addressModeU);
    auto llglAddressModeV = GetLLGLAddressMode(texture->addressModeV);
    auto llglAddressModeW = GetLLGLAddressMode(texture->addressModeW);
    auto llglMinFilter = GetLLGLFilter(texture->minFilter);
    auto llglMagFilter = GetLLGLFilter(texture->magFilter);
    auto llglMipMapFilter = GetLLGLFilter(texture->mipMapFilter);

    LLGL::SamplerDescriptor samplerDescriptor{
            .addressModeU = llglAddressModeU,
            .addressModeV = llglAddressModeV,
            .addressModeW = llglAddressModeW,
            .minFilter = llglMinFilter,
            .magFilter = llglMagFilter,
            .mipMapFilter = llglMipMapFilter,
            .maxAnisotropy = 16
    };

    return renderSystem->CreateSampler(samplerDescriptor);
}

std::vector<std::unique_ptr<Mesh>>
GetMeshes(const std::shared_ptr<LLGL::RenderSystem> &renderSystem, const DAsset::BufferCollection &bufferCollection,
          const DAsset::Mesh &mesh) {
    std::vector<std::unique_ptr<Mesh>> meshes{};
    for (const auto &meshPart: mesh.meshParts) {
        // index buffer
        std::optional<uint32_t> nVertices = std::nullopt, nIndices = std::nullopt;
        LLGL::Buffer *indexBuffer = nullptr;
        {
            auto indexBufferViewOpt = meshPart.indexBufferView;
            if (indexBufferViewOpt.has_value()) {
                auto indexBufferView = indexBufferViewOpt.value();
                auto bufferPtr = indexBufferView.buffer;
                nIndices = static_cast<uint32_t>(indexBufferView.byteLength /
                                                 DAsset::GetSize(indexBufferView.dataType,
                                                                 indexBufferView.componentType
                                                 )
                );
                if (indexBufferView.componentType != DAsset::ComponentType::SCALAR) {
                    RAISE_EXCEPTION(errorhandling::IllegalArgumentException,
                                    "Index buffer component type must be scalar");
                }
                // Create buffer
                {
                    LLGL::BufferDescriptor bufferDesc{
                            .size = indexBufferView.byteLength,
                            .stride = static_cast<uint32_t>(indexBufferView.byteStride),
                            .format = GetBufferFormat(indexBufferView.dataType,
                                                      indexBufferView.componentType),
                            .bindFlags = LLGL::BindFlags::IndexBuffer,
                    };
                    auto data = &bufferPtr->data[indexBufferView.byteOffset];
                    indexBuffer = renderSystem->CreateBuffer(bufferDesc, data);
                }
            }
        }
        // buffer array
        LLGL::BufferArray *bufferArray;
        std::vector<LLGL::Buffer *> buffers{};
        std::vector<LLGL::VertexFormat> vertexFormats{};
        {
            for (const auto &entry: meshPart.attributeBufferViews) {
                auto attributeType = entry.first;
                auto bufferView = entry.second;
                uint8_t *data = &bufferView.buffer->data[bufferView.byteOffset];

                if (attributeType == DAsset::AttributeType::POSITION) {
                    nVertices = static_cast<uint32_t>(bufferView.byteLength /
                                                      DAsset::GetSize(bufferView.dataType,
                                                                      bufferView.componentType
                                                      )
                    );
                }

                LLGL::VertexFormat vertexFormat{};
                // Create vertex format
                {
                    auto attributeTypeName = DAsset::GetAttributeTypeName(attributeType);
                    // to lower case
                    for (auto &c: attributeTypeName) {
                        c = std::tolower(c);
                    }
                    auto requiredComponentType = DAsset::GetRequiredComponentTypeForAttribute(attributeType);
                    auto actualComponentType = bufferView.componentType;
                    if (actualComponentType != requiredComponentType) {
                        throw std::runtime_error("Component type of attribute " + attributeTypeName +
                                                 " (" + DAsset::GetComponentTypeName(actualComponentType) +
                                                 ") does not match required component type (" +
                                                 DAsset::GetComponentTypeName(requiredComponentType) + ")"
                        );
                    }
                    vertexFormat.AppendAttribute(
                            {
                                    attributeTypeName.c_str(),
                                    GetBufferFormat(
                                            bufferView.dataType,
                                            requiredComponentType
                                    ),
                                    GetLocationIndex(attributeType)
                            }
                    );
                    vertexFormat.SetSlot(vertexFormats.size());
                    vertexFormats.push_back(vertexFormat);
                }
                // Create vertex attribute buffer
                LLGL::Buffer *buffer;
                {
                    LLGL::BufferDescriptor bufferDesc{
                            .size = bufferView.byteLength,
                            .stride = static_cast<uint32_t>(bufferView.byteStride),
                            .format = GetBufferFormat(
                                    bufferView.dataType,
                                    bufferView.componentType
                            ),
                            .bindFlags = LLGL::BindFlags::VertexBuffer,
                            .vertexAttribs = vertexFormat.attributes
                    };
                    buffer = renderSystem->CreateBuffer(bufferDesc, data);
                    buffers.push_back(buffer);
                }
            }
            if (buffers.empty()) {
                RAISE_EXCEPTION(errorhandling::IllegalStateException, "No vertex attributes found in mesh part");
            }
            bufferArray = renderSystem->CreateBufferArray(buffers.size(), buffers.data());
        }

        // Material
        std::shared_ptr<Material> material = std::make_shared<Material>(renderSystem);
        {
            auto dAssetMaterial = meshPart.material;
            material->name = dAssetMaterial->name;
            material->albedoFactor = dAssetMaterial->albedoFactor;
            material->roughnessFactor = dAssetMaterial->roughnessFactor;
            material->metalnessFactor = dAssetMaterial->metalnessFactor;
            material->ambientOcclusionFactor = dAssetMaterial->ambientOcclusionFactor;
            material->normalScale = dAssetMaterial->normalScale;

            // Textures and samplers are created depending on whether the material has the given texture (ptr != nullptr)
            // Respective samplers must be non-null when respective textures are non-null
            material->albedoTexture = LoadOptionalTexture(renderSystem, dAssetMaterial->albedoTexture);
            material->normalTexture = LoadOptionalTexture(renderSystem, dAssetMaterial->normalTexture);
            material->rmaTexture = LoadOptionalTexture(renderSystem,
                                                       dAssetMaterial->metallicRoughnessAmbientOcclusionTexture);

            material->albedoSampler = CreateOptionalSampler(renderSystem, dAssetMaterial->albedoTexture);
            material->normalSampler = CreateOptionalSampler(renderSystem, dAssetMaterial->normalTexture);
            material->rmaSampler = CreateOptionalSampler(renderSystem,
                                                         dAssetMaterial->metallicRoughnessAmbientOcclusionTexture);
            // Create texture present flag buffer
            {
                uint32_t flags = 0;
                if (material->albedoTexture != nullptr) {
                    flags |= (1 << 0);
                }
                if (material->normalTexture != nullptr) {
                    flags |= (1 << 1);
                }
                if (material->rmaTexture != nullptr) {
                    flags |= (1 << 2);
                }
                LLGL::BufferDescriptor bufferDescriptor = {
                        .size = sizeof(MaterialShaderState)
                };
                MaterialShaderState materialShaderState = {
                        .texturePresentStates = flags,
                        .albedoFactor = material->albedoFactor,
                        .roughnessFactor = material->roughnessFactor,
                        .metalnessFactor = material->metalnessFactor,
                        .ambientOcclusionFactor = material->ambientOcclusionFactor,
                        .normalScale = material->normalScale
                };
                material->texturePresentFlagsBuffer = renderSystem->CreateBuffer(bufferDescriptor, &materialShaderState);
            }

        }
        meshes.push_back(
                std::make_unique<Mesh>(GetMeshRenderMode(meshPart.renderMode),
                                       renderSystem,
                                       nVertices, nIndices,
                                       indexBuffer, buffers, bufferArray,
                                       vertexFormats,
                                       material
                )
        );
    }
    return meshes;
}


Asset *AssetLoader::LoadAsset(const std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                              const std::unique_ptr<Stream::DataReadStream> &assetDataStream) {
    auto dAsset = DAsset::ReadAsset(assetDataStream);
    Asset *asset = new Asset{};
    std::queue<DAsset::Node> nodes;
    nodes.push(dAsset.rootNode);
    while (!nodes.empty()) {
        auto node = nodes.front();
        nodes.pop();
        auto meshes = GetMeshes(renderSystem, dAsset.bufferCollection, node.mesh);
        glm::mat4 modelMatrix = glm::identity<glm::mat4>();
        modelMatrix = glm::translate(modelMatrix, node.translation);
        modelMatrix *= glm::mat4_cast(node.rotation);
        modelMatrix = glm::scale(modelMatrix, node.scale);

        std::unique_ptr<Node> nodePtr = std::make_unique<Node>(modelMatrix, *asset);
        for (auto &mesh: meshes) {
            nodePtr->addMesh(mesh);
        }
        asset->nodes.push_back(std::move(nodePtr));
        for (const auto &item: node.children) {
            nodes.push(item);
        }
    }
    return asset;
}
