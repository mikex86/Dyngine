#pragma once

#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <Stream/FileDataWriteStream.hpp>
#include <map>

namespace DAsset {

    enum class DataType {
        BYTE,
        UNSIGNED_BYTE,
        SHORT,
        UNSIGNED_SHORT,
        INT,
        UNSIGNED_INT,
        FLOAT,
        DOUBLE
    };

    enum class ComponentType {
        SCALAR,
        VEC2,
        VEC3,
        VEC4,
        MAT2,
        MAT3,
        MAT4
    };

    struct Buffer {
        uint64_t bufferIndex;
        std::vector<uint8_t> data;

        Buffer(uint64_t bufferIndex, const std::vector<uint8_t> &data);
    };

    struct BufferCollection {
        std::vector<std::shared_ptr<Buffer>> buffers;

        std::optional<uint64_t> find(const std::shared_ptr<DAsset::Buffer> &buffer) const;

        std::shared_ptr<Buffer> getBuffer(uint64_t index) const;
    };

    struct BufferView {
        uint64_t byteOffset;
        uint64_t byteLength;
        uint64_t byteStride;
        DataType dataType;
        ComponentType componentType;
        std::shared_ptr<Buffer> buffer;
    };

    enum class RenderMode {
        POINTS,
        LINES,
        LINE_LOOP,
        LINE_STRIP,
        TRIANGLES,
        TRIANGLE_STRIP,
        TRIANGLE_FAN
    };

    enum class AttributeType {
        POSITION,
        NORMAL,
        TANGENT,
        TEX_COORD,
        COLOR,
        JOINTS,
        WEIGHTS
    };

    struct MeshPart {
        RenderMode renderMode;
        std::optional<BufferView> indexBufferView;
        std::map<AttributeType, BufferView> attributeBufferViews;
    };

    struct Mesh {
        std::vector<MeshPart> meshParts;
    };

    struct Node {
        std::string name;
        glm::vec3 translation;
        glm::quat rotation;
        glm::vec3 scale;
        Mesh mesh;
        std::vector<Node> children;
    };

    struct Asset {
        Node rootNode;
        BufferCollection bufferCollection;
    };

    void WriteAsset(const Asset &asset, const std::unique_ptr<Stream::DataWriteStream> &stream);

    DAsset::Asset ReadAsset(const std::unique_ptr<Stream::DataReadStream> &stream);

    std::string GetAttributeTypeName(const AttributeType type);

    std::string GetDataTypeName(const DataType type);

    std::string GetComponentTypeName(const ComponentType type);

    std::string GetRenderModeName(RenderMode renderMode);

    uint64_t GetSize(DataType dataType, ComponentType componentType);

}