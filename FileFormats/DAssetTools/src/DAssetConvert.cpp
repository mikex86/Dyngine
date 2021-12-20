#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <tiny_gltf.h>
#include "DAsset/Asset.hpp"
#include <iostream>
#include <ErrorHandling/IllegalStateException.hpp>

#include <Stream/FileDataWriteStream.hpp>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>


DAsset::Asset FromTinyGLTF(const tinygltf::Model &model);

DAsset::AttributeType GetAttributeType(const std::string &attributeName);

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " input.glb output.dasset" << std::endl;
        return 1;
    }
    using namespace tinygltf;
    Model model;
    TinyGLTF loader;
    std::string err;
    std::string warn;
    bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, argv[1]);
    if (!warn.empty()) {
        std::cerr << "Warnings: " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cerr << "Errors: " << err << std::endl;
    }
    if (!ret) {
        std::cerr << "Failed to load glTF" << std::endl;
        return 1;
    }
    DAsset::Asset asset = FromTinyGLTF(model);
    DAsset::WriteAsset(asset, Stream::FileDataWriteStream::Open(argv[2]));
}

DAsset::DataType GetDataType(int gltfComponentType) {
    switch (gltfComponentType) {
        case TINYGLTF_COMPONENT_TYPE_BYTE:
            return DAsset::DataType::BYTE;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            return DAsset::DataType::UNSIGNED_SHORT;
        case TINYGLTF_COMPONENT_TYPE_SHORT:
            return DAsset::DataType::SHORT;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            return DAsset::DataType::UNSIGNED_SHORT;
        case TINYGLTF_COMPONENT_TYPE_INT:
            return DAsset::DataType::INT;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            return DAsset::DataType::UNSIGNED_SHORT;
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            return DAsset::DataType::FLOAT;
        default:
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "Unknown gltf component type");
    }
}


DAsset::ComponentType GetComponentType(int gltfType) {
    switch (gltfType) {
        case TINYGLTF_TYPE_SCALAR:
            return DAsset::ComponentType::SCALAR;
        case TINYGLTF_TYPE_VEC2:
            return DAsset::ComponentType::VEC2;
        case TINYGLTF_TYPE_VEC3:
            return DAsset::ComponentType::VEC3;
        case TINYGLTF_TYPE_VEC4:
            return DAsset::ComponentType::VEC4;
        case TINYGLTF_TYPE_MAT2:
            return DAsset::ComponentType::MAT2;
        case TINYGLTF_TYPE_MAT3:
            return DAsset::ComponentType::MAT3;
        case TINYGLTF_TYPE_MAT4:
            return DAsset::ComponentType::MAT4;
        default:
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "Unknown gltf type");
    }
}

DAsset::RenderMode GetRenderMode(int gltfMode) {
    switch (gltfMode) {
        case TINYGLTF_MODE_LINE:
            return DAsset::RenderMode::LINES;
        case TINYGLTF_MODE_LINE_LOOP:
            return DAsset::RenderMode::LINE_LOOP;
        case TINYGLTF_MODE_LINE_STRIP:
            return DAsset::RenderMode::LINE_STRIP;
        case TINYGLTF_MODE_TRIANGLE_FAN:
            return DAsset::RenderMode::TRIANGLE_FAN;
        case TINYGLTF_MODE_TRIANGLE_STRIP:
            return DAsset::RenderMode::TRIANGLE_STRIP;
        case TINYGLTF_MODE_TRIANGLES:
            return DAsset::RenderMode::TRIANGLES;
        case TINYGLTF_MODE_POINTS:
            return DAsset::RenderMode::POINTS;
        default:
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "Unknown render mode: " + std::to_string(gltfMode));
    }
}

DAsset::AttributeType GetAttributeType(const std::string &attributeName) {
    if (attributeName == "POSITION")
        return DAsset::AttributeType::POSITION;
    if (attributeName == "NORMAL")
        return DAsset::AttributeType::NORMAL;
    if (attributeName == "TANGENT")
        return DAsset::AttributeType::TANGENT;
    if (attributeName == "TEXCOORD_0")
        return DAsset::AttributeType::TEX_COORD;
    if (attributeName == "COLOR_0")
        return DAsset::AttributeType::COLOR;
    if (attributeName == "JOINTS_0")
        return DAsset::AttributeType::JOINTS;
    if (attributeName == "WEIGHTS_0")
        return DAsset::AttributeType::WEIGHTS;
    RAISE_EXCEPTION(errorhandling::IllegalStateException, "Unknown attribute type: " + attributeName);
}


DAsset::Mesh
FromGLTFMesh(DAsset::BufferCollection &bufferCollection, const tinygltf::Model &model, const tinygltf::Mesh &mesh) {
    DAsset::Mesh dAssetMesh{};
    for (const auto &primitive: mesh.primitives) {
        DAsset::BufferView indexBufferView;
        // Index buffer view
        if (primitive.indices != -1) {
            auto accessor = model.accessors[primitive.indices];
            auto bufferView = model.bufferViews[accessor.bufferView];
            auto dAssetBuffer = bufferCollection.buffers[bufferView.buffer];
            indexBufferView = DAsset::BufferView{bufferView.byteOffset + accessor.byteOffset, accessor.count,
                                                 bufferView.byteStride,
                                                 GetDataType(accessor.componentType),
                                                 GetComponentType(accessor.type),
                                                 dAssetBuffer};
        }
        std::map<DAsset::AttributeType, DAsset::BufferView> attributeBufferViews{};
        // other vertex attributes. eg. position, normal, etc.
        for (auto[targetName, accessorIndex]: primitive.attributes) {
            auto accessor = model.accessors[accessorIndex];
            auto bufferView = model.bufferViews[accessor.bufferView];
            auto dAssetBuffer = bufferCollection.buffers[bufferView.buffer];
            auto attributeType = GetAttributeType(targetName);
            attributeBufferViews[attributeType] = {bufferView.byteOffset + accessor.byteOffset, accessor.count,
                                                bufferView.byteStride,
                                                GetDataType(accessor.componentType),
                                                GetComponentType(accessor.type),
                                                dAssetBuffer};
        }
        DAsset::MeshPart dAssetMeshPart{GetRenderMode(primitive.mode), indexBufferView, attributeBufferViews};
        dAssetMesh.meshParts.push_back(dAssetMeshPart);
    }
    return dAssetMesh;
}

DAsset::Node
FromGLTFNode(DAsset::BufferCollection &bufferCollection, const tinygltf::Model &model, const tinygltf::Node &gltfNode) {
    DAsset::Node dAssetNode{};
    dAssetNode.name = gltfNode.name;

    // Get translation, rotation and scale
    {
        if (gltfNode.matrix.size() == 16) {
            glm::mat4 matrix = glm::make_mat4(&gltfNode.matrix[0]);
            glm::mat4 transformation;
            glm::vec3 scale;
            glm::quat rotation;
            glm::vec3 translation;
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(transformation, scale, rotation, translation, skew, perspective);
            dAssetNode.translation = translation;
            dAssetNode.rotation = rotation;
            dAssetNode.scale = scale;
        } else {
            if (gltfNode.translation.size() == 3) {
                dAssetNode.translation = glm::vec3(gltfNode.translation[0], gltfNode.translation[1],
                                                   gltfNode.translation[2]);
            }
            if (gltfNode.rotation.size() == 4) {
                dAssetNode.rotation = glm::quat(gltfNode.rotation[0], gltfNode.rotation[1], gltfNode.rotation[2],
                                                gltfNode.rotation[3]);
            }
            if (gltfNode.scale.size() == 3) {
                dAssetNode.scale = glm::vec3(gltfNode.scale[0], gltfNode.scale[1], gltfNode.scale[2]);
            }
        }
    }
    // Get mesh
    {
        if (gltfNode.mesh >= 0) {
            const tinygltf::Mesh &gltfMesh = model.meshes[gltfNode.mesh];
            dAssetNode.mesh = FromGLTFMesh(bufferCollection, model, gltfMesh);
        }
    }

    for (const auto &nodeIndex: gltfNode.children) {
        auto childGltfNode = model.nodes[nodeIndex];
        dAssetNode.children.push_back(FromGLTFNode(bufferCollection, model, childGltfNode));
    }
    return dAssetNode;
}

DAsset::BufferCollection FromGLTFBuffers(const tinygltf::Model &model) {
    DAsset::BufferCollection bufferCollection{};
    uint64_t bufferIndex = 0;
    for (const auto &buffer: model.buffers) {
        bufferCollection.buffers.push_back(
                std::make_shared<DAsset::Buffer>(bufferIndex, buffer.data)
        );
        bufferIndex++;
    }
    return bufferCollection;
}

DAsset::Asset FromTinyGLTF(const tinygltf::Model &model) {
    DAsset::Asset asset{};
    if (model.nodes.empty()) {
        return asset;
    }
    auto bufferCollection = FromGLTFBuffers(model);
    asset.bufferCollection = bufferCollection;
    DAsset::Node rootNode{};
    auto gltfRootNodes = model.scenes[0].nodes;
    for (const auto &gltfRootNode: gltfRootNodes) {
        auto gltfNode = model.nodes[gltfRootNode];
        rootNode = FromGLTFNode(bufferCollection, model, gltfNode);
    }
    asset.rootNode = rootNode;
    return asset;
}
