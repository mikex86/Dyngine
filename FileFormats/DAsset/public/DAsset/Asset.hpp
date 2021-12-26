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
        uint64_t bufferId;
        std::vector<uint8_t> data;

        Buffer(uint64_t bufferId, const std::vector<uint8_t> &data);
    };

    struct BufferCollection {
        std::vector<std::shared_ptr<Buffer>> buffers;

        std::optional<uint64_t> find(const DAsset::Buffer &buffer) const;

        std::optional<std::shared_ptr<Buffer>> getBuffer(uint64_t bufferId) const;

        std::shared_ptr<Buffer> newBuffer(const uint8_t *data, size_t length);
    };

    struct BufferView {
        uint64_t byteOffset;
        uint64_t byteLength;
        uint64_t byteStride;
        DataType dataType;
        ComponentType componentType;
        std::shared_ptr<Buffer> buffer;
    };

    enum TextureFilter {
        NEAREST, LINEAR
    };

    enum TextureAddressMode {
        REPEAT, MIRROR, CLAMP, BORDER, MIRROR_ONCE
    };

    struct Texture {
        uint64_t textureId;
        int32_t width, height;
        int32_t channels;
        uint32_t bitDepth;
        TextureFilter textureFilter = TextureFilter::LINEAR;
        TextureAddressMode addressMode = TextureAddressMode::REPEAT;

        Texture(uint64_t textureId);

        BufferView bufferView;
    };

    struct TextureCollection {
        std::vector<std::shared_ptr<Texture>> textures;

        std::optional<std::shared_ptr<Texture>> getTexture(uint64_t textureId) const;

        std::optional<uint64_t> find(const DAsset::Texture &texture) const;

        std::shared_ptr<Texture> newTexture();
    };

    struct Material {
        uint64_t materialId;
        glm::vec4 albedoFactor;
        float roughnessFactor;
        float metalnessFactor;
        float ambientOcclusionFactor;
        float normalScale;

        std::optional<std::shared_ptr<Texture>> albedoTexture = std::nullopt;
        std::optional<std::shared_ptr<Texture>> normalTexture = std::nullopt;
        std::optional<std::shared_ptr<Texture>> metallicRoughnessAmbientOcclusionTexture = std::nullopt;

        Material(uint64_t materialId);
    };

    struct MaterialCollection {
        std::vector<std::shared_ptr<Material>> materials;

        std::optional<std::shared_ptr<Material>> getMaterial(uint64_t materialId) const;

        std::optional<uint64_t> find(const Material &material) const;

        std::shared_ptr<Material> newMaterial();
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
        std::shared_ptr<DAsset::Material> material;
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
        TextureCollection textureCollection;
        MaterialCollection materialCollection;
    };

    void WriteAsset(const Asset &asset, const std::unique_ptr<Stream::DataWriteStream> &stream);

    DAsset::Asset ReadAsset(const std::unique_ptr<Stream::DataReadStream> &stream);

    std::string GetAttributeTypeName(const AttributeType type);

    std::string GetDataTypeName(const DataType type);

    std::string GetComponentTypeName(const ComponentType type);

    std::string GetRenderModeName(RenderMode renderMode);

    uint64_t GetSize(DataType dataType, ComponentType componentType);

    std::string GetTextureAddressModeName(TextureAddressMode mode);

    std::string GetTextureFilterName(TextureFilter filter);
}