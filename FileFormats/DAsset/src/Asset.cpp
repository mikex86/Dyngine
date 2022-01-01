#include <DAsset/Asset.hpp>
#include "ErrorHandling/IllegalArgumentException.hpp"
#include <ErrorHandling/IllegalStateException.hpp>
#include <optional>

void WriteBufferView(const DAsset::BufferCollection &bufferCollection, const DAsset::BufferView &bufferView,
                     const std::unique_ptr<Stream::DataWriteStream> &stream) {
    stream->writeInt64(bufferView.byteOffset);
    stream->writeInt64(bufferView.byteLength);
    stream->writeInt64(bufferView.byteStride);
    stream->writeInt8(static_cast<int8_t>(bufferView.dataType));
    stream->writeInt8(static_cast<int8_t>(bufferView.componentType));
    auto bufferIdOpt = bufferCollection.find(*bufferView.buffer);
    if (!bufferIdOpt.has_value()) {
        RAISE_EXCEPTION(errorhandling::IllegalStateException, "BufferView references unknown buffer");
    }
    stream->writeInt64(bufferIdOpt.value());
}

void
WriteMeshPart(const DAsset::BufferCollection &bufferCollection, const DAsset::MaterialCollection &materialCollection,
              const DAsset::MeshPart &meshPart, const std::unique_ptr<Stream::DataWriteStream> &stream) {

    // Write render mode
    stream->writeInt8(static_cast<int8_t>(meshPart.renderMode));

    // Write optional index buffer view
    if (meshPart.indexBufferView.has_value()) {
        stream->writeInt8(1);
        WriteBufferView(bufferCollection, meshPart.indexBufferView.value(), stream);
    } else {
        stream->writeInt8(0);
    }

    // Write attribute buffer views
    stream->writeInt64(meshPart.attributeBufferViews.size());
    for (const auto &entry: meshPart.attributeBufferViews) {
        stream->writeInt8(static_cast<int8_t>(entry.first));
        WriteBufferView(bufferCollection, entry.second, stream);
    }

    // Write material reference
    {
        auto materialIdOpt = materialCollection.find(*meshPart.material);
        if (!materialIdOpt.has_value()) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "Material references unknown material");
        }
        stream->writeInt64(materialIdOpt.value());
    }
}

void WriteMesh(const DAsset::BufferCollection &bufferCollection, const DAsset::MaterialCollection &materialCollection,
               const DAsset::Mesh &mesh, const std::unique_ptr<Stream::DataWriteStream> &stream) {
    stream->writeInt64(mesh.meshParts.size());
    for (const auto &meshPart: mesh.meshParts) {
        WriteMeshPart(bufferCollection, materialCollection, meshPart, stream);
    }
}

void WriteNode(const DAsset::BufferCollection &bufferCollection, const DAsset::MaterialCollection &materialCollection,
               const DAsset::Node &node, const std::unique_ptr<Stream::DataWriteStream> &stream) {
    stream->writeString(node.name);
    stream->writeFloat32(node.translation.x);
    stream->writeFloat32(node.translation.y);
    stream->writeFloat32(node.translation.z);
    stream->writeFloat32(node.rotation.x);
    stream->writeFloat32(node.rotation.y);
    stream->writeFloat32(node.rotation.z);
    stream->writeFloat32(node.rotation.w);
    stream->writeFloat32(node.scale.x);
    stream->writeFloat32(node.scale.y);
    stream->writeFloat32(node.scale.z);
    WriteMesh(bufferCollection, materialCollection, node.mesh, stream);
    stream->writeInt64(node.children.size());
    for (const auto &child: node.children) {
        WriteNode(bufferCollection, materialCollection, child, stream);
    }
}

void WriteBufferCollection(const DAsset::BufferCollection &collection,
                           const std::unique_ptr<Stream::DataWriteStream> &stream) {
    stream->writeInt64(collection.buffers.size());
    for (const auto &buffer: collection.buffers) {
        stream->writeInt64(buffer->bufferId);
        stream->writeInt64(buffer->data.size());
        stream->writeBuffer(buffer->data.data(), buffer->data.size());
    }
}

void WriteTextureCollection(const DAsset::BufferCollection &bufferCollection,
                            const DAsset::TextureCollection &textureCollection,
                            const std::unique_ptr<Stream::DataWriteStream> &stream) {
    stream->writeInt64(textureCollection.textures.size());
    for (const auto &texture: textureCollection.textures) {
        stream->writeInt64(texture->textureId);
        stream->writeInt32(texture->width);
        stream->writeInt32(texture->height);
        stream->writeInt32(texture->channels);
        stream->writeInt32(texture->bitDepth);
        stream->writeInt8(texture->minFilter);
        stream->writeInt8(texture->magFilter);
        stream->writeInt8(texture->mipMapFilter);
        stream->writeInt8(texture->addressModeU);
        stream->writeInt8(texture->addressModeV);
        stream->writeInt8(texture->addressModeW);
        WriteBufferView(bufferCollection, texture->bufferView, stream);
    }
}

void WriteOptionalTextureReference(std::optional<std::shared_ptr<DAsset::Texture>> optionalTexture,
                                   const DAsset::TextureCollection &textureCollection,
                                   const std::unique_ptr<Stream::DataWriteStream> &stream) {
    if (optionalTexture.has_value()) {
        stream->writeInt8(1);
        auto textureIdOpt = textureCollection.find(*optionalTexture.value());
        if (!textureIdOpt.has_value()) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "Unknown texture referenced");
        }
        stream->writeInt64(textureIdOpt.value());
    } else {
        stream->writeInt8(0);
    }
}

void WriteMaterialCollection(const DAsset::MaterialCollection &materialCollection,
                             const DAsset::TextureCollection &textureCollection,
                             const std::unique_ptr<Stream::DataWriteStream> &stream) {
    stream->writeInt64(materialCollection.materials.size());
    for (const auto &material: materialCollection.materials) {
        stream->writeInt64(material->materialId);
        stream->writeString(material->name);
        stream->writeFloat32(material->albedoFactor.x);
        stream->writeFloat32(material->albedoFactor.y);
        stream->writeFloat32(material->albedoFactor.z);
        stream->writeFloat32(material->albedoFactor.w);
        stream->writeFloat32(material->roughnessFactor);
        stream->writeFloat32(material->metalnessFactor);
        stream->writeFloat32(material->ambientOcclusionFactor);
        stream->writeFloat32(material->normalScale);
        WriteOptionalTextureReference(material->albedoTexture, textureCollection, stream);
        WriteOptionalTextureReference(material->normalTexture, textureCollection, stream);
        WriteOptionalTextureReference(material->metallicRoughnessAmbientOcclusionTexture, textureCollection, stream);
    }
}

void DAsset::WriteAsset(const DAsset::Asset &asset, const std::unique_ptr<Stream::DataWriteStream> &stream) {
    WriteBufferCollection(asset.bufferCollection, stream);
    WriteTextureCollection(asset.bufferCollection, asset.textureCollection, stream);
    WriteMaterialCollection(asset.materialCollection, asset.textureCollection, stream);
    WriteNode(asset.bufferCollection, asset.materialCollection, asset.rootNode, stream);
}

DAsset::BufferView
ReadBufferView(const DAsset::BufferCollection &bufferCollection,
               const std::unique_ptr<Stream::DataReadStream> &stream) {
    DAsset::BufferView bufferView{};
    bufferView.byteOffset = stream->readInt64();
    bufferView.byteLength = stream->readInt64();
    bufferView.byteStride = stream->readInt64();
    bufferView.dataType = static_cast<DAsset::DataType>(stream->readInt8());
    bufferView.componentType = static_cast<DAsset::ComponentType>(stream->readInt8());
    uint64_t bufferIndex = stream->readInt64();
    if (bufferIndex >= bufferCollection.buffers.size()) {
        RAISE_EXCEPTION(errorhandling::IllegalStateException, "Mesh references unknown buffer");
    }
    auto bufferOpt = bufferCollection.getBuffer(bufferIndex);
    if (!bufferOpt.has_value()) {
        RAISE_EXCEPTION(errorhandling::IllegalStateException, "Mesh references unknown buffer");
    }
    auto buffer = bufferOpt.value();
    bufferView.buffer = buffer;
    return bufferView;
}

DAsset::Mesh
ReadMesh(const DAsset::BufferCollection &bufferCollection, const DAsset::MaterialCollection &materialCollection,
         const std::unique_ptr<Stream::DataReadStream> &stream) {
    DAsset::Mesh mesh{};
    mesh.meshParts.resize(stream->readInt64());
    for (auto &meshPart: mesh.meshParts) {
        meshPart.renderMode = static_cast<DAsset::RenderMode>(stream->readInt8());
        // Read optional index buffer view
        if (stream->readUint8() == 1) {
            meshPart.indexBufferView = ReadBufferView(bufferCollection, stream);
        }
        // Read attribute buffer views
        auto nAttributes = stream->readInt64();
        for (uint64_t i = 0; i < nAttributes; ++i) {
            auto attributeType = static_cast<DAsset::AttributeType>(stream->readInt8());
            meshPart.attributeBufferViews[attributeType] = ReadBufferView(bufferCollection, stream);
        }
        // Read material reference
        auto materialIndex = stream->readInt64();
        auto materialOpt = materialCollection.getMaterial(materialIndex);
        if (!materialOpt.has_value()) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "MeshPart references unknown material");
        }
        meshPart.material = materialOpt.value();
    }
    return mesh;
}

DAsset::Node
ReadNode(const DAsset::BufferCollection &bufferCollection, const DAsset::MaterialCollection &materialCollection,
         const std::unique_ptr<Stream::DataReadStream> &stream) {
    DAsset::Node node{};
    node.name = stream->readString();
    node.translation.x = stream->readFloat32();
    node.translation.y = stream->readFloat32();
    node.translation.z = stream->readFloat32();
    node.rotation.x = stream->readFloat32();
    node.rotation.y = stream->readFloat32();
    node.rotation.z = stream->readFloat32();
    node.rotation.w = stream->readFloat32();
    node.scale.x = stream->readFloat32();
    node.scale.y = stream->readFloat32();
    node.scale.z = stream->readFloat32();
    node.mesh = ReadMesh(bufferCollection, materialCollection, stream);
    auto nChildren = stream->readInt64();
    for (uint64_t i = 0; i < nChildren; ++i) {
        node.children.push_back(ReadNode(bufferCollection, materialCollection, stream));
    }
    return node;
}

DAsset::BufferCollection ReadBufferCollection(const std::unique_ptr<Stream::DataReadStream> &stream) {
    auto bufferCount = stream->readInt64();
    DAsset::BufferCollection collection;
    collection.buffers = std::vector<std::shared_ptr<DAsset::Buffer>>(bufferCount);
    for (uint64_t bufferIndex = 0u; bufferIndex < bufferCount; ++bufferIndex) {
        auto bufferId = stream->readInt64();
        auto bufferSize = stream->readInt64();
        auto data = std::vector<uint8_t>(bufferSize);
        stream->read(data.data(), bufferSize);
        collection.buffers[bufferIndex] = std::make_shared<DAsset::Buffer>(bufferId, data);
    }
    return collection;
}

DAsset::TextureCollection ReadTextureCollection(const DAsset::BufferCollection &bufferCollection,
                                                const std::unique_ptr<Stream::DataReadStream> &stream) {
    auto textureCount = stream->readInt64();
    DAsset::TextureCollection textureCollection{};
    textureCollection.textures = std::vector<std::shared_ptr<DAsset::Texture>>(textureCount);
    for (uint64_t textureIndex = 0u; textureIndex < textureCount; ++textureIndex) {
        auto textureId = stream->readInt64();
        auto width = stream->readInt32();
        auto height = stream->readInt32();
        auto channels = stream->readInt32();
        auto bitDepth = stream->readInt32();
        auto minFilter = static_cast<DAsset::SamplerFilter>(stream->readInt8());
        auto magFilter = static_cast<DAsset::SamplerFilter>(stream->readInt8());
        auto mipMapFilter = static_cast<DAsset::SamplerFilter>(stream->readInt8());

        auto addressModeU = static_cast<DAsset::SamplerAddressMode>(stream->readInt8());
        auto addressModeV = static_cast<DAsset::SamplerAddressMode>(stream->readInt8());
        auto addressModeW = static_cast<DAsset::SamplerAddressMode>(stream->readInt8());

        auto dataSize = width * height * channels * (bitDepth / 8);
        auto bufferView = ReadBufferView(bufferCollection, stream);
        auto texture = std::make_shared<DAsset::Texture>(textureId);
        texture->width = width;
        texture->height = height;
        texture->channels = channels;
        texture->bitDepth = bitDepth;
        texture->bufferView = bufferView;
        texture->minFilter = minFilter;
        texture->magFilter = magFilter;
        texture->mipMapFilter = mipMapFilter;
        texture->addressModeU = addressModeU;
        texture->addressModeV = addressModeV;
        texture->addressModeW = addressModeW;
        textureCollection.textures[textureIndex] = texture;
    }
    return textureCollection;
}

std::optional<std::shared_ptr<DAsset::Texture>> ReadOptionalTexture(const DAsset::TextureCollection &textureCollection,
                                                                    const std::unique_ptr<Stream::DataReadStream> &stream) {
    auto optionalState = stream->readInt8();
    if (optionalState == 0) {
        return std::nullopt;
    }
    auto textureId = stream->readInt64();
    auto textureOpt = textureCollection.getTexture(textureId);
    if (!textureOpt.has_value()) {
        RAISE_EXCEPTION(errorhandling::IllegalStateException, "Texture references unknown texture");
    }
    return textureOpt;
}

DAsset::MaterialCollection ReadMaterialCollection(const DAsset::TextureCollection &textureCollection,
                                                  const std::unique_ptr<Stream::DataReadStream> &stream) {
    auto materialCount = stream->readInt64();
    DAsset::MaterialCollection materialCollection{};
    materialCollection.materials = std::vector<std::shared_ptr<DAsset::Material>>(materialCount);
    for (uint64_t materialIndex = 0u; materialIndex < materialCount; ++materialIndex) {
        auto materialId = stream->readInt64();
        auto materialName = stream->readString();

        auto material = std::make_shared<DAsset::Material>(materialId);
        material->name = materialName;
        material->albedoFactor =
                {stream->readFloat32(), stream->readFloat32(), stream->readFloat32(), stream->readFloat32()};
        material->roughnessFactor = stream->readFloat32();
        material->metalnessFactor = stream->readFloat32();
        material->ambientOcclusionFactor = stream->readFloat32();
        material->normalScale = stream->readFloat32();

        material->albedoTexture = ReadOptionalTexture(textureCollection, stream);
        material->normalTexture = ReadOptionalTexture(textureCollection, stream);
        material->metallicRoughnessAmbientOcclusionTexture = ReadOptionalTexture(textureCollection, stream);
        materialCollection.materials[materialIndex] = material;
    }
    return materialCollection;
}

DAsset::Asset DAsset::ReadAsset(const std::unique_ptr<Stream::DataReadStream> &stream) {
    DAsset::Asset asset{};
    auto bufferCollection = ReadBufferCollection(stream);
    auto textureCollection = ReadTextureCollection(bufferCollection, stream);
    auto materialCollection = ReadMaterialCollection(textureCollection, stream);
    asset.bufferCollection = bufferCollection;
    asset.textureCollection = textureCollection;
    asset.materialCollection = materialCollection;
    asset.rootNode = ReadNode(bufferCollection, materialCollection, stream);
    return asset;
}

std::string DAsset::GetAttributeTypeName(const DAsset::AttributeType type) {
    switch (type) {
        case AttributeType::POSITION:
            return "POSITION";
        case AttributeType::NORMAL:
            return "NORMAL";
        case AttributeType::TANGENT:
            return "TANGENT";
        case AttributeType::TEX_COORD:
            return "TEX_COORD";
        case AttributeType::COLOR:
            return "COLOR";
        case AttributeType::JOINTS:
            return "JOINTS";
        case AttributeType::WEIGHTS:
            return "WEIGHTS";
        default:
            return "UNKNOWN";
    }
}

std::string DAsset::GetDataTypeName(const DAsset::DataType type) {
    switch (type) {
        case DataType::BYTE:
            return "BYTE";
        case DataType::UNSIGNED_BYTE:
            return "UNSIGNED_BYTE";
        case DataType::SHORT:
            return "SHORT";
        case DataType::UNSIGNED_SHORT:
            return "UNSIGNED_SHORT";
        case DataType::INT:
            return "INT";
        case DataType::UNSIGNED_INT:
            return "UNSIGNED_INT";
        case DataType::FLOAT:
            return "FLOAT";
        default:
            return "UNKNOWN";
    }
}

std::string DAsset::GetComponentTypeName(const DAsset::ComponentType type) {
    switch (type) {
        case ComponentType::SCALAR:
            return "SCALAR";
        case ComponentType::VEC2:
            return "VEC2";
        case ComponentType::VEC3:
            return "VEC3";
        case ComponentType::VEC4:
            return "VEC4";
        case ComponentType::MAT2:
            return "MAT2";
        case ComponentType::MAT3:
            return "MAT3";
        case ComponentType::MAT4:
            return "MAT4";
        default:
            return "UNKNOWN";
    }
}

std::string DAsset::GetRenderModeName(DAsset::RenderMode renderMode) {
    switch (renderMode) {
        case RenderMode::TRIANGLES:
            return "TRIANGLES";
        case RenderMode::TRIANGLE_STRIP:
            return "TRIANGLE_STRIP";
        case RenderMode::TRIANGLE_FAN:
            return "TRIANGLE_FAN";
        case RenderMode::LINES:
            return "LINES";
        case RenderMode::LINE_STRIP:
            return "LINE_STRIP";
        case RenderMode::LINE_LOOP:
            return "LINE_LOOP";
        case RenderMode::POINTS:
            return "POINTS";
        default:
            return "UNKNOWN";
    }
}

uint64_t DAsset::GetSize(DAsset::DataType dataType, DAsset::ComponentType componentType) {
    uint64_t stride = 0;
    switch (dataType) {
        case DAsset::DataType::BYTE:
        case DAsset::DataType::UNSIGNED_BYTE:
            stride = 1;
            break;
        case DAsset::DataType::SHORT:
        case DAsset::DataType::UNSIGNED_SHORT:
            stride = 2;
            break;
        case DAsset::DataType::INT:
        case DAsset::DataType::UNSIGNED_INT:
        case DAsset::DataType::FLOAT:
            stride = 4;
            break;
        default:
            RAISE_EXCEPTION(errorhandling::IllegalArgumentException, "Unknown data type");
    }
    uint64_t componentCount = 0;
    switch (componentType) {
        case DAsset::ComponentType::SCALAR:
            componentCount = 1;
            break;
        case DAsset::ComponentType::VEC2:
            componentCount = 2;
            break;
        case DAsset::ComponentType::VEC3:
            componentCount = 3;
            break;
        case DAsset::ComponentType::VEC4:
            componentCount = 4;
            break;
        case DAsset::ComponentType::MAT2:
            componentCount = 4;
            break;
        case DAsset::ComponentType::MAT3:
            componentCount = 9;
            break;
        case DAsset::ComponentType::MAT4:
            componentCount = 16;
            break;
        default:
            RAISE_EXCEPTION(errorhandling::IllegalArgumentException, "Unknown component type");
    }
    return stride * componentCount;
}

std::string DAsset::GetTextureAddressModeName(DAsset::SamplerAddressMode mode) {
    switch (mode) {
        case SamplerAddressMode::REPEAT:
            return "REPEAT";
        case SamplerAddressMode::MIRROR:
            return "MIRROR";
        case SamplerAddressMode::CLAMP:
            return "CLAMP";
        case SamplerAddressMode::BORDER:
            return "BORDER";
        case SamplerAddressMode::MIRROR_ONCE:
            return "MIRROR_ONCE";
        default:
            return "UNKNOWN";
    }
}

std::string DAsset::GetTextureFilterName(DAsset::SamplerFilter filter) {
    switch (filter) {
        case SamplerFilter::LINEAR:
            return "LINEAR";
        case SamplerFilter::NEAREST:
            return "NEAREST";
        default:
            return "UNKNOWN";
    }
}

DAsset::ComponentType DAsset::GetRequiredComponentTypeForAttribute(DAsset::AttributeType attributeType) {
    switch (attributeType) {
        case AttributeType::POSITION:
            return ComponentType::VEC3;
        case AttributeType::NORMAL:
            return ComponentType::VEC3;
        case AttributeType::TANGENT:
            return ComponentType::VEC4;
        case AttributeType::TEX_COORD:
            return ComponentType::VEC2;
        case AttributeType::COLOR:
            return ComponentType::VEC4;
        case AttributeType::JOINTS:
            return ComponentType::VEC4;
        case AttributeType::WEIGHTS:
            return ComponentType::VEC4;
        default:
            // TODO: FIND OUT MISSING CASES
            RAISE_EXCEPTION(errorhandling::IllegalArgumentException,
                            "Don't know required attribute type for attribute type " +
                            GetAttributeTypeName(attributeType));
    }
}

std::optional<uint64_t> DAsset::BufferCollection::find(const DAsset::Buffer &buffer) const {
    for (uint64_t i = 0; i < buffers.size(); i++) {
        if (buffers[i]->bufferId == buffer.bufferId) {
            return i;
        }
    }
    return std::nullopt;
}

std::optional<std::shared_ptr<DAsset::Buffer>> DAsset::BufferCollection::getBuffer(uint64_t bufferId) const {
    for (auto &buffer: buffers) {
        if (buffer->bufferId == bufferId) {
            return buffer;
        }
    }
    return std::nullopt;
}

std::shared_ptr<DAsset::Buffer> DAsset::BufferCollection::newBuffer(const uint8_t *data, size_t length) {
    std::vector<uint8_t> bufferData(length);
    std::copy(data, data + length, bufferData.begin());
    auto buffer = std::make_shared<DAsset::Buffer>(buffers.size(), bufferData);
    buffers.push_back(buffer);
    return buffer;
}

DAsset::Buffer::Buffer(uint64_t bufferId, const std::vector<uint8_t> &data) : bufferId(bufferId), data(data) {
}

std::optional<std::shared_ptr<DAsset::Material>> DAsset::MaterialCollection::getMaterial(uint64_t materialId) const {
    for (auto &material: materials) {
        if (material->materialId == materialId) {
            return material;
        }
    }
    return std::nullopt;
}

std::optional<uint64_t> DAsset::MaterialCollection::find(const DAsset::Material &material) const {
    for (uint64_t i = 0; i < materials.size(); i++) {
        if (materials[i]->materialId == material.materialId) {
            return i;
        }
    }
    return std::nullopt;
}

std::shared_ptr<DAsset::Material> DAsset::MaterialCollection::newMaterial() {
    auto material = std::make_shared<DAsset::Material>(materials.size());
    materials.push_back(material);
    return material;
}

DAsset::Material::Material(uint64_t materialId) : materialId(materialId) {
}

std::optional<std::shared_ptr<DAsset::Texture>> DAsset::TextureCollection::getTexture(uint64_t textureId) const {
    for (auto &texture: textures) {
        if (texture->textureId == textureId) {
            return texture;
        }
    }
    return std::nullopt;
}

std::optional<uint64_t> DAsset::TextureCollection::find(const DAsset::Texture &texture) const {
    for (uint64_t i = 0; i < textures.size(); i++) {
        if (textures[i]->textureId == texture.textureId) {
            return i;
        }
    }
    return std::nullopt;
}

std::shared_ptr<DAsset::Texture> DAsset::TextureCollection::newTexture() {
    auto texture = std::make_shared<DAsset::Texture>(textures.size());
    textures.push_back(texture);
    return texture;
}

DAsset::Texture::Texture(uint64_t textureId) : textureId(textureId) {
}
