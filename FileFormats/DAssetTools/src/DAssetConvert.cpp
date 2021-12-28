#define TINYGLTF_IMPLEMENTATION

#include <stb_image.h>
#include <stb_image_write.h>

#undef STB_IMAGE_IMPLEMENTATION
#undef STB_IMAGE_WRITE_IMPLEMENTATION

#include <tiny_gltf.h>
#include <DAsset/Asset.hpp>
#include <iostream>
#include <ErrorHandling/IllegalStateException.hpp>

#include <Stream/FileDataWriteStream.hpp>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <queue>
#include <map>

DAsset::Asset FromTinyGLTF(const tinygltf::Model &model);

DAsset::AttributeType GetAttributeType(const std::string &attributeName);

struct ConverterState {
private:
    /**
     * Key: (RoughnessMetallicTextureIndex << 32 | OcclusionTextureIndex)
     * Value: rmaTextureId
     */
    std::map<uint64_t, uint64_t> rmaTexturesLookupToId{};

    /**
     * Key: glTF texture index
     * Value: DAsset texture id
     */
    std::map<uint64_t, uint64_t> textureIndexToIdMapping{};

    /**
     * Key: glTF material index
     * value: DAsset material id
     */
    std::map<uint64_t, uint64_t> materialIndexToIdMapping{};

public:

    std::optional<uint64_t> getRMATextureId(uint64_t key) {
        auto it = rmaTexturesLookupToId.find(key);
        if (it == rmaTexturesLookupToId.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    void setRMATextureId(uint64_t key, uint64_t value) {
        rmaTexturesLookupToId[key] = value;
    }

    std::optional<uint64_t> getTextureId(uint64_t gltfTextureIndex) {
        auto it = textureIndexToIdMapping.find(gltfTextureIndex);
        if (it == textureIndexToIdMapping.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    void setTextureId(uint64_t gltfTextureIndex, uint64_t dAssetTextureId) {
        textureIndexToIdMapping[gltfTextureIndex] = dAssetTextureId;
    }

    std::optional<uint64_t> getMaterialId(uint64_t gltfMaterialIndex) {
        auto it = materialIndexToIdMapping.find(gltfMaterialIndex);
        if (it == materialIndexToIdMapping.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    void setMaterialId(uint64_t gltfMaterialIndex, uint64_t dAssetMaterialId) {
        materialIndexToIdMapping[gltfMaterialIndex] = dAssetMaterialId;
    }

};

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

stbi_uc *ReadTextureImageData(const tinygltf::Model &model, uint64_t textureIndex, int32_t &width, int32_t &height,
                              int32_t &channels, int requiredComp) {
    auto &gltfTexture = model.textures[textureIndex];
    auto &gltfImage = model.images[gltfTexture.source];
    auto &gltfBufferView = model.bufferViews[gltfImage.bufferView];
    auto &gltfBuffer = model.buffers[gltfBufferView.buffer];
    auto bufferData = gltfBuffer.data;
    auto bufferSize = gltfBufferView.byteLength;
    auto bufferOffset = gltfBufferView.byteOffset;
    if (gltfBufferView.byteStride != 0) {
        RAISE_EXCEPTION(errorhandling::IllegalStateException, "Stride not supported on texture buffers");
    }
    channels = requiredComp;
    return stbi_load_from_memory(&bufferData[bufferOffset], bufferSize,
                                 &width, &height, nullptr,
                                 requiredComp);
}

struct custom_stbi_mem_context {
    size_t last_pos;
    uint8_t *buffer;
    size_t bufferLength;
};

void MemoryDecode(void *context, void *data, int size) {
    static const int bufferSize = 1048560;
    custom_stbi_mem_context *c = (custom_stbi_mem_context *) context;
    if (c->buffer == nullptr) {
        c->buffer = new uint8_t[size];
        c->bufferLength = size;
    } else if (c->last_pos + size >= c->bufferLength) {
        c->bufferLength = c->last_pos + size + bufferSize;
        auto newBuffer = new uint8_t[c->bufferLength];
        memcpy(newBuffer, c->buffer, c->last_pos);
        delete[] c->buffer;
        c->buffer = newBuffer;
    }
    char *src = (char *) data;
    int cur_pos = c->last_pos;
    for (int i = 0; i < size; i++) {
        c->buffer[cur_pos++] = src[i];
    }
    c->last_pos = cur_pos;
}

uint8_t *JpegCompress(const uint8_t *imageData, uint32_t width, uint32_t height, uint32_t channels, uint32_t quality,
                      size_t &compressedSize) {
    custom_stbi_mem_context context{};
    int result = stbi_write_jpg_to_func(MemoryDecode, &context, width, height, channels, imageData, quality);
    compressedSize = context.last_pos;
    return context.buffer;
}

DAsset::SamplerFilter GetTextureFilter(int gltfFilter) {
    switch (gltfFilter) {
        case TINYGLTF_TEXTURE_FILTER_NEAREST:
        case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
        case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
            return DAsset::SamplerFilter::NEAREST;
        case TINYGLTF_TEXTURE_FILTER_LINEAR:
        case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
        case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
            return DAsset::SamplerFilter::LINEAR;
        default:
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "Unknown gltf filter: " + std::to_string(gltfFilter));
    }
}

DAsset::SamplerFilter GetMipmapFilter(int gltfFilter) {
    switch (gltfFilter) {
        case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
        case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
            return DAsset::SamplerFilter::NEAREST;
        case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
        case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
            return DAsset::SamplerFilter::LINEAR;
        default:
            return DAsset::LINEAR;
    }
}

DAsset::SamplerAddressMode GetAddressMode(int gltfWrap) {
    switch (gltfWrap) {
        case TINYGLTF_TEXTURE_WRAP_REPEAT:
            return DAsset::SamplerAddressMode::REPEAT;
        case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
            return DAsset::SamplerAddressMode::CLAMP;
        case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
            return DAsset::SamplerAddressMode::REPEAT;
        default:
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "Unknown gltf wrap: " + std::to_string(gltfWrap));
    }
}

std::optional<std::shared_ptr<DAsset::Texture>>
GetOrMakeTexture(DAsset::Asset &asset, const tinygltf::Model &model, uint64_t textureIndex,
                 ConverterState &converterState) {
    auto &textureCollection = asset.textureCollection;
    auto &bufferCollection = asset.bufferCollection;
    if (textureIndex == -1) {
        return std::nullopt;
    }
    auto textureIdOpt = converterState.getTextureId(textureIndex);
    if (!textureIdOpt.has_value()) {
        auto texture = textureCollection.newTexture();
        converterState.setTextureId(textureIndex, texture->textureId);

        // TODO: Support other formats than jpeg

        auto &gltfTexture = model.textures[textureIndex];
        auto sampler = model.samplers[gltfTexture.sampler];

        texture->addressModeU = GetAddressMode(sampler.wrapS);
        texture->addressModeV = GetAddressMode(sampler.wrapT);
        texture->addressModeW = DAsset::SamplerAddressMode::CLAMP;

        texture->minFilter = GetTextureFilter(sampler.minFilter);
        texture->magFilter = GetTextureFilter(sampler.magFilter);
        texture->mipMapFilter = GetMipmapFilter(sampler.magFilter);

        stbi_uc *imageData = ReadTextureImageData(model, textureIndex, texture->width, texture->height,
                                                  texture->channels, STBI_rgb);
        texture->bitDepth = 8;
        if (imageData == nullptr) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "Failed to load texture data");
        }

        uint32_t imageWidth = texture->width, imageHeight = texture->height, imageChannels = texture->channels;
        uint64_t imageDataLength = imageWidth * imageWidth * imageChannels;

        size_t compressedImageDataLength{};
        auto compressedImageData = JpegCompress(
                imageData, imageWidth, imageHeight,
                imageChannels, 80, // TODO: remove hardcoded quality
                compressedImageDataLength
        );
        delete[] imageData;

        auto buffer = bufferCollection.newBuffer(compressedImageData, compressedImageDataLength);
        delete[] compressedImageData;

        texture->bufferView = {
                .byteOffset = 0,
                .byteLength = compressedImageDataLength,
                .byteStride = 0,
                .dataType = DAsset::DataType::UNSIGNED_BYTE,
                .componentType = DAsset::ComponentType::VEC4,
                .buffer = buffer,
        };
        return texture;
    }
    auto textureId = textureIdOpt.value();
    auto textureOpt = textureCollection.getTexture(textureId);
    if (!textureOpt.has_value()) {
        RAISE_EXCEPTION(errorhandling::IllegalStateException,
                        "Illegal texture collection state. Texture not found, but ConverterState entry exists");
    }
    return textureOpt.value();
}

std::optional<std::shared_ptr<DAsset::Texture>>
GetOrMakeRMATexture(DAsset::Asset &asset, const tinygltf::Model &model, const tinygltf::Material &material,
                    ConverterState &converterState) {
    auto &textureCollection = asset.textureCollection;
    auto &bufferCollection = asset.bufferCollection;
    uint64_t metallicRoughnessTextureIndex = material.pbrMetallicRoughness.metallicRoughnessTexture.index;
    uint64_t occlusionTextureIndex = material.occlusionTexture.index;
    uint64_t lookupKey = metallicRoughnessTextureIndex << 32 || occlusionTextureIndex;

    auto rmaTextureIdOpt = converterState.getRMATextureId(lookupKey);
    if (!rmaTextureIdOpt.has_value()) {
        stbi_uc *metallicRoughnessImageData = nullptr;

        // Metallic roughness texture
        int32_t metallicRoughnessWidth{}, metallicRoughnessHeight{}, metallicRoughnessChannels{};
        if (metallicRoughnessTextureIndex != -1) {
            auto &gltfMetallicRoughnessTexture = model.textures[metallicRoughnessTextureIndex];

            metallicRoughnessImageData = ReadTextureImageData(model, metallicRoughnessTextureIndex,
                                                              metallicRoughnessWidth, metallicRoughnessHeight,
                                                              metallicRoughnessChannels, 3);
            if (metallicRoughnessImageData == nullptr) {
                RAISE_EXCEPTION(errorhandling::IllegalStateException, "Failed to load roughness-metallic texture data");
            }

            uint64_t metallicRoughnessImageDataLength =
                    metallicRoughnessWidth * metallicRoughnessHeight * 3 * sizeof(uint8_t);
        }

        stbi_uc *occlusionImageData = nullptr;

        // Occlusion texture
        int32_t occlusionWidth{}, occlusionHeight{}, occlusionChannels{};
        if (occlusionTextureIndex != -1) {
            auto &gltfOcclusionTexture = model.textures[occlusionTextureIndex];
            occlusionImageData = ReadTextureImageData(model, occlusionTextureIndex, occlusionWidth,
                                                      occlusionHeight, occlusionChannels, 1);
            if (occlusionImageData == nullptr) {
                RAISE_EXCEPTION(errorhandling::IllegalStateException, "Failed to load occlusion texture data");
            }
            uint64_t occlusionImageDataLength = occlusionWidth * occlusionHeight * 1 * sizeof(uint8_t);
        }
        if (metallicRoughnessImageData != nullptr && occlusionImageData != nullptr) {
            if (occlusionWidth != metallicRoughnessWidth || occlusionHeight != metallicRoughnessHeight) {
                RAISE_EXCEPTION(errorhandling::IllegalStateException,
                                "Occlusion texture dimensions do not match roughness-metallic texture dimensions");
            }
        } else if (metallicRoughnessImageData == nullptr && occlusionImageData == nullptr) {
            return std::nullopt;
        }

        uint64_t rmaImageWidth{};
        uint64_t rmaImageHeight{};

        if (metallicRoughnessImageData != nullptr) {
            rmaImageWidth = metallicRoughnessWidth;
            rmaImageHeight = metallicRoughnessHeight;
        } else if (occlusionImageData != nullptr) {
            rmaImageWidth = occlusionWidth;
            rmaImageHeight = occlusionHeight;
        }

        uint64_t rmaImageChannels = 3;
        uint64_t rmaImageDataLength = rmaImageWidth * rmaImageHeight * rmaImageChannels * sizeof(uint8_t);
        // (R = roughness, G = Metalness, B = Ambient Occlusion)
        uint8_t *rmaImageData = new stbi_uc[rmaImageDataLength];
        for (uint64_t i = 0; i < rmaImageDataLength; i++) {
            uint64_t pixel = i / rmaImageChannels;
            uint64_t channel = i % rmaImageChannels;
            if (channel == 0) {
                // Roughness is in the green channel of the metallicRoughness texture.
                if (metallicRoughnessImageData == nullptr) {
                    rmaImageData[i] = 0;
                } else {
                    rmaImageData[i] = metallicRoughnessImageData[pixel * 3 + 1];
                }
            } else if (channel == 1) {
                // Metalness is in the blue channel of the metallicRoughness texture.
                if (metallicRoughnessImageData == nullptr) {
                    rmaImageData[i] = 0;
                } else {
                    rmaImageData[i] = metallicRoughnessImageData[pixel * 3 + 2];
                }
            } else if (channel == 2) {
                // Ambient Occlusion is in the red channel of the occlusion texture.
                if (occlusionImageData == nullptr) {
                    rmaImageData[i] = 0;
                } else {
                    rmaImageData[i] = occlusionImageData[pixel];
                }
            }
        }
        size_t rmaCompressedImageDataLength{};
        auto rmaCompressedImageData = JpegCompress(
                rmaImageData, rmaImageWidth, rmaImageHeight,
                rmaImageChannels, 50, // TODO: remove hardcoded quality
                rmaCompressedImageDataLength
        );
        delete[] rmaImageData;

        auto rmaTexture = textureCollection.newTexture();
        converterState.setRMATextureId(lookupKey, rmaTexture->textureId);
        rmaTexture->width = rmaImageWidth;
        rmaTexture->height = rmaImageHeight;
        rmaTexture->channels = rmaImageChannels;
        rmaTexture->bitDepth = 8;

        rmaTexture->addressModeU = DAsset::CLAMP;
        rmaTexture->addressModeV = DAsset::CLAMP;
        rmaTexture->addressModeW = DAsset::CLAMP;

        rmaTexture->minFilter = DAsset::LINEAR;
        rmaTexture->magFilter = DAsset::LINEAR;
        rmaTexture->mipMapFilter = DAsset::LINEAR;

        rmaTexture->bufferView = DAsset::BufferView{
                .byteOffset = 0,
                .byteLength = rmaCompressedImageDataLength,
                .byteStride = 0,
                .buffer = bufferCollection.newBuffer(rmaCompressedImageData, rmaCompressedImageDataLength)
        };
        delete[] rmaCompressedImageData;
        delete[] metallicRoughnessImageData;
        delete[] occlusionImageData;
        return rmaTexture;
    }
    auto rmaTextureId = rmaTextureIdOpt.value();
    auto rmaTextureOpt = textureCollection.getTexture(rmaTextureId);
    if (!rmaTextureOpt.has_value()) {
        RAISE_EXCEPTION(errorhandling::IllegalStateException,
                        "Illegal Texture collection state. Texture not found, but ConverterState entry exists");
    }
    return rmaTextureOpt.value();
}

std::shared_ptr<DAsset::Material>
GetOrMakeMaterial(DAsset::Asset &asset, const tinygltf::Model &model, uint64_t materialIndex,
                  ConverterState &converterState) {
    auto &materialCollection = asset.materialCollection;

    auto materialIdOpt = converterState.getMaterialId(materialIndex);
    if (!materialIdOpt.has_value()) {
        auto material = materialCollection.newMaterial();
        auto gltfMaterial = model.materials[materialIndex];
        auto gltfPbr = gltfMaterial.pbrMetallicRoughness;
        material->albedoFactor = glm::vec4(gltfPbr.baseColorFactor[0], gltfPbr.baseColorFactor[1],
                                           gltfPbr.baseColorFactor[2], gltfPbr.baseColorFactor[3]);
        material->roughnessFactor = gltfPbr.roughnessFactor;
        material->metalnessFactor = gltfPbr.metallicFactor;
        material->ambientOcclusionFactor = gltfMaterial.occlusionTexture.strength;
        material->normalScale = gltfMaterial.normalTexture.scale;

        // Albedo texture
        {
            auto albedoTextureOpt = GetOrMakeTexture(asset, model, gltfPbr.baseColorTexture.index, converterState);
            if (albedoTextureOpt.has_value()) {
                material->albedoTexture = albedoTextureOpt.value();
            }
        }
        // Normal texture
        {
            auto normalTextureOpt = GetOrMakeTexture(asset, model, gltfMaterial.normalTexture.index, converterState);
            if (normalTextureOpt.has_value()) {
                material->normalTexture = normalTextureOpt.value();
            }
        }
        // RMA texture
        {
            auto rmaTextureOpt = GetOrMakeRMATexture(asset, model, gltfMaterial, converterState);
            if (rmaTextureOpt.has_value()) {
                material->metallicRoughnessAmbientOcclusionTexture = rmaTextureOpt.value();
            }
        }
        return material;
    }
    auto materialId = materialIdOpt.value();
    auto materialOpt = materialCollection.getMaterial(materialId);
    if (!materialOpt.has_value()) {
        RAISE_EXCEPTION(errorhandling::IllegalStateException,
                        "Illegal material collection state. Material not found, but entry in converter state exists");
    }
    return *materialOpt;
}


DAsset::BufferView
MakeBufferView(DAsset::BufferCollection &bufferCollection, const tinygltf::Model &model, uint64_t accessorIndex,
               ConverterState &converterState) {
    auto accessor = model.accessors[accessorIndex];
    auto bufferView = model.bufferViews[accessor.bufferView];

    auto dataType = GetDataType(accessor.componentType);
    auto componentType = GetComponentType(accessor.type);
    auto elementSize = DAsset::GetSize(dataType, componentType);

    auto dAssetBuffer = bufferCollection.newBuffer(
            &model.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset],
            bufferView.byteLength
    );

    return {0,
            accessor.count * elementSize,
            bufferView.byteStride, // TODO: un-stride array to avoid storing unreferenced data
            dataType,
            componentType,
            dAssetBuffer
    };
}

DAsset::Mesh
FromGLTFMesh(DAsset::Asset &asset, const tinygltf::Model &model, const tinygltf::Mesh &mesh,
             ConverterState &converterState) {
    auto &bufferCollection = asset.bufferCollection;

    DAsset::Mesh dAssetMesh{};
    for (const auto &primitive: mesh.primitives) {
        DAsset::BufferView indexBufferView;

        // TODO: Reference already existing buffer in buffer views when possible

        // Create index buffer + view
        if (primitive.indices != -1) {
            auto accessorIndex = primitive.indices;
            indexBufferView = MakeBufferView(bufferCollection, model, accessorIndex, converterState);
        }

        std::map<DAsset::AttributeType, DAsset::BufferView> attributeBufferViews{};
        // Create buffers + views for other vertex attributes. eg. position, normal, etc.
        for (auto[targetName, accessorIndex]: primitive.attributes) {
            auto attributeType = GetAttributeType(targetName);
            attributeBufferViews[attributeType] = MakeBufferView(bufferCollection, model, accessorIndex,
                                                                 converterState);
        }
        // Add Material
        std::shared_ptr<DAsset::Material> material;
        {
            auto materialId = primitive.material;
            material = GetOrMakeMaterial(asset, model, materialId, converterState);
        }
        DAsset::MeshPart dAssetMeshPart{GetRenderMode(primitive.mode), indexBufferView, attributeBufferViews,
                                        material};
        dAssetMesh.meshParts.push_back(dAssetMeshPart);
    }
    return dAssetMesh;
}

DAsset::Node
FromGLTFNode(DAsset::Asset &asset, const tinygltf::Model &model, const tinygltf::Node &gltfNode,
             ConverterState &converterState) {
    auto bufferCollection = asset.bufferCollection;
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
            } else {
                dAssetNode.scale = glm::vec3(1.0f);
            }
        }
    }
    // Get mesh
    {
        if (gltfNode.mesh >= 0) {
            const tinygltf::Mesh &gltfMesh = model.meshes[gltfNode.mesh];
            dAssetNode.mesh = FromGLTFMesh(asset, model, gltfMesh, converterState);
        }
    }

    for (const auto &nodeIndex: gltfNode.children) {
        auto childGltfNode = model.nodes[nodeIndex];
        dAssetNode.children.push_back(FromGLTFNode(asset, model, childGltfNode, converterState));
    }
    return dAssetNode;
}

DAsset::Asset FromTinyGLTF(const tinygltf::Model &model) {
    ConverterState converterState{};
    DAsset::Asset asset{};
    if (model.nodes.empty()) {
        return asset;
    }
    asset.bufferCollection = {};
    asset.textureCollection = {};
    asset.materialCollection = {};
    DAsset::Node rootNode{
            .name = "synthetic_root",
            .translation = glm::vec3(0.0f, 0.0f, 0.0f),
            .rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            .scale = glm::vec3(1.0f, 1.0f, 1.0f)
    };
    auto gltfRootNodes = model.scenes[0].nodes;
    for (const auto &gltfRootNode: gltfRootNodes) {
        auto gltfNode = model.nodes[gltfRootNode];
        // Adds to bufferCollection, textureCollection and materialCollection
        rootNode.children.push_back(FromGLTFNode(asset, model, gltfNode, converterState));
    }
    asset.rootNode = rootNode;
    return asset;
}
