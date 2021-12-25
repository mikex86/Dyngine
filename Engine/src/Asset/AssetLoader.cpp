#include <Asset/AssetLoader.hpp>
#include <DAsset/Asset.hpp>
#include <ErrorHandling/IllegalArgumentException.hpp>
#include <ErrorHandling/IllegalStateException.hpp>
#include <glm/gtx/quaternion.hpp>
#include <optional>
#include <queue>

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

std::vector<std::unique_ptr<Mesh>>
GetMeshes(std::shared_ptr<LLGL::RenderSystem> renderSystem, const DAsset::BufferCollection &bufferCollection,
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
                // Create buffer
                {
                    LLGL::BufferDescriptor bufferDesc{
                            .size = indexBufferView.byteLength,
                            .format = GetBufferFormat(indexBufferView.dataType,
                                                      indexBufferView.componentType),
                            .bindFlags = LLGL::BindFlags::IndexBuffer
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
            uint32_t locationIndex = 0;
            for (const auto &entry: meshPart.attributeBufferViews) {
                auto attributeType = entry.first;
                auto bufferView = entry.second;
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
                    std::transform(attributeTypeName.begin(), attributeTypeName.end(),
                                   attributeTypeName.begin(),
                                   [](unsigned char c) { return std::tolower(c); }
                    );
                    vertexFormat.AppendAttribute(
                            {
                                    attributeTypeName.c_str(),
                                    GetBufferFormat(
                                            bufferView.dataType,
                                            bufferView.componentType
                                    ),
                                    locationIndex
                            }
                    );
                    vertexFormat.SetSlot(locationIndex);
                    vertexFormats.push_back(vertexFormat);
                }
                // Create buffer
                LLGL::Buffer *buffer;
                {
                    LLGL::BufferDescriptor bufferDesc{
                            .size = bufferView.byteLength,
                            .stride = static_cast<uint32_t>(bufferView.byteOffset),
                            .format = GetBufferFormat(
                                    bufferView.dataType,
                                    bufferView.componentType
                            ),
                            .bindFlags = LLGL::BindFlags::VertexBuffer,
                            .vertexAttribs = vertexFormat.attributes
                    };
                    auto data = &bufferView.buffer->data[bufferView.byteOffset];
                    buffer = renderSystem->CreateBuffer(bufferDesc, data);
                    buffers.push_back(buffer);
                }
                locationIndex++;
            }
            if (buffers.empty()) {
                RAISE_EXCEPTION(errorhandling::IllegalStateException, "No vertex attributes found in mesh part");
            }
            bufferArray = renderSystem->CreateBufferArray(buffers.size(), buffers.data());
        }
        meshes.push_back(
                std::make_unique<Mesh>(GetMeshRenderMode(meshPart.renderMode),
                                       renderSystem,
                                       nVertices, nIndices,
                                       indexBuffer, buffers, bufferArray,
                                       vertexFormats
                )
        );
    }
    return meshes;
}

Asset *AssetLoader::LoadAsset(std::shared_ptr<LLGL::RenderSystem> renderSystem,
                              const std::unique_ptr<Stream::DataReadStream> &assetDataStream) {
    auto dAsset = DAsset::ReadAsset(assetDataStream);
    Asset *asset = new Asset{};
    std::queue<DAsset::Node> nodes;
    nodes.push(dAsset.rootNode);
    while (!nodes.empty()) {
        auto node = nodes.front();
        nodes.pop();
        auto meshes = GetMeshes(renderSystem, dAsset.bufferCollection, node.mesh);
        std::unique_ptr<Node> nodePtr = std::make_unique<Node>();
        glm::mat4 modelMatrix = glm::identity<glm::mat4>();
        modelMatrix = glm::translate(modelMatrix, node.translation);
        modelMatrix *= glm::mat4_cast(node.rotation);
        modelMatrix = glm::scale(modelMatrix, node.scale);
        nodePtr->modelMatrix = modelMatrix;
        for (auto &mesh: meshes) {
            nodePtr->meshes.push_back(std::move(mesh));
        }
        asset->nodes.push_back(std::move(nodePtr));
        for (const auto &item: node.children) {
            nodes.push(item);
        }
    }
    return asset;
}
