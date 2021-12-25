#include "DAsset/Asset.hpp"
#include "ErrorHandling/IllegalArgumentException.hpp"
#include <ErrorHandling/IllegalStateException.hpp>
#include <optional>

#define MAX_NODE_NAME_LENGTH 32

void WriteBufferView(const DAsset::BufferCollection &bufferCollection, const DAsset::BufferView &bufferView,
                     const std::unique_ptr<Stream::DataWriteStream> &stream) {
    stream->writeInt64(bufferView.byteOffset);
    stream->writeInt64(bufferView.byteLength);
    stream->writeInt64(bufferView.byteStride);
    stream->writeInt8(static_cast<int8_t>(bufferView.dataType));
    stream->writeInt8(static_cast<int8_t>(bufferView.componentType));
    auto indexOpt = bufferCollection.find(bufferView.buffer);
    if (!indexOpt) {
        RAISE_EXCEPTION(errorhandling::IllegalStateException, "BufferView references unknown buffer");
    }
    stream->writeInt64(indexOpt.value());
}

void WriteMeshPart(const DAsset::BufferCollection &bufferCollection, const DAsset::MeshPart &meshPart,
                   const std::unique_ptr<Stream::DataWriteStream> &stream) {
    stream->writeInt8(static_cast<int8_t>(meshPart.renderMode));
    if (meshPart.indexBufferView.has_value()) {
        stream->writeInt8(1);
        WriteBufferView(bufferCollection, meshPart.indexBufferView.value(), stream);
    } else {
        stream->writeInt8(0);
    }
    stream->writeInt64(meshPart.attributeBufferViews.size());
    for (const auto &entry: meshPart.attributeBufferViews) {
        stream->writeInt8(static_cast<int8_t>(entry.first));
        WriteBufferView(bufferCollection, entry.second, stream);
    }
}

void WriteMesh(const DAsset::BufferCollection &bufferCollection, const DAsset::Mesh &mesh,
               const std::unique_ptr<Stream::DataWriteStream> &stream) {
    stream->writeInt64(mesh.meshParts.size());
    for (const auto &meshPart: mesh.meshParts) {
        WriteMeshPart(bufferCollection, meshPart, stream);
    }
}

void WriteNode(const DAsset::BufferCollection &bufferCollection, const DAsset::Node &node,
               const std::unique_ptr<Stream::DataWriteStream> &stream) {
    stream->writeFixedString(node.name, MAX_NODE_NAME_LENGTH);
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
    WriteMesh(bufferCollection, node.mesh, stream);
    stream->writeInt64(node.children.size());
    for (const auto &child: node.children) {
        WriteNode(bufferCollection, child, stream);
    }
}

void WriteBufferCollection(const DAsset::BufferCollection &collection,
                           const std::unique_ptr<Stream::DataWriteStream> &stream) {
    stream->writeInt64(collection.buffers.size());
    for (const auto &buffer: collection.buffers) {
        stream->writeInt64(buffer->data.size());
        stream->writeBuffer(buffer->data.data(), buffer->data.size());
    }
}

void DAsset::WriteAsset(const DAsset::Asset &asset, const std::unique_ptr<Stream::DataWriteStream> &stream) {
    WriteBufferCollection(asset.bufferCollection, stream);
    WriteNode(asset.bufferCollection, asset.rootNode, stream);
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
    auto buffer = bufferCollection.getBuffer(bufferIndex);
    bufferView.buffer = buffer;
    return bufferView;
}

DAsset::Mesh
ReadMesh(const DAsset::BufferCollection &bufferCollection, const std::unique_ptr<Stream::DataReadStream> &stream) {
    DAsset::Mesh mesh{};
    mesh.meshParts.resize(stream->readInt64());
    for (auto &meshPart: mesh.meshParts) {
        meshPart.renderMode = static_cast<DAsset::RenderMode>(stream->readInt8());
        if (stream->readUint8() == 1) {
            meshPart.indexBufferView = ReadBufferView(bufferCollection, stream);
        }
        auto nAttributes = stream->readInt64();
        for (uint64_t i = 0; i < nAttributes; ++i) {
            auto attributeType = static_cast<DAsset::AttributeType>(stream->readInt8());
            meshPart.attributeBufferViews[attributeType] = ReadBufferView(bufferCollection, stream);
        }
    }
    return mesh;
}

DAsset::Node
ReadNode(const DAsset::BufferCollection &bufferCollection, const std::unique_ptr<Stream::DataReadStream> &stream) {
    DAsset::Node node{};
    node.name = stream->readFixedString(MAX_NODE_NAME_LENGTH);
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
    node.mesh = ReadMesh(bufferCollection, stream);
    auto nChildren = stream->readInt64();
    for (uint64_t i = 0; i < nChildren; ++i) {
        node.children.push_back(ReadNode(bufferCollection, stream));
    }
    return node;
}

DAsset::BufferCollection ReadBufferCollection(const std::unique_ptr<Stream::DataReadStream> &stream) {
    auto bufferCount = stream->readInt64();
    DAsset::BufferCollection collection;
    collection.buffers = std::vector<std::shared_ptr<DAsset::Buffer>>(bufferCount);
    for (uint64_t bufferIndex = 0u; bufferIndex < bufferCount; ++bufferIndex) {
        auto bufferSize = stream->readInt64();
        auto data = std::vector<uint8_t>(bufferSize);
        stream->read(data.data(), bufferSize);
        collection.buffers[bufferIndex] = std::make_shared<DAsset::Buffer>(bufferIndex, data);
    }
    return collection;
}

DAsset::Asset DAsset::ReadAsset(const std::unique_ptr<Stream::DataReadStream> &stream) {
    DAsset::Asset asset{};
    auto bufferCollection = ReadBufferCollection(stream);
    asset.bufferCollection = bufferCollection;
    asset.rootNode = ReadNode(bufferCollection, stream);
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

std::optional<uint64_t> DAsset::BufferCollection::find(const std::shared_ptr<DAsset::Buffer> &buffer) const {
    for (uint64_t i = 0; i < buffers.size(); i++) {
        if (buffers[i]->bufferIndex == buffer->bufferIndex) {
            return i;
        }
    }
    return std::nullopt;
}

std::shared_ptr<DAsset::Buffer> DAsset::BufferCollection::getBuffer(uint64_t index) const {
    for (auto &buffer: buffers) {
        if (buffer->bufferIndex == index) {
            return buffer;
        }
    }
    return nullptr;
}

DAsset::Buffer::Buffer(uint64_t bufferIndex, const std::vector<uint8_t> &data) : bufferIndex(bufferIndex), data(data) {}
