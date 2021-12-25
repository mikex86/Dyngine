#include <DShader/DShader.hpp>

namespace DShader {

    ShaderVariant ReadDShaderVariant(const std::unique_ptr<Stream::DataReadStream> &stream) {
        ShaderVariant variant{};
        variant.type = static_cast<ShaderType>(stream->readInt32());
        variant.api = static_cast<ShaderApi>(stream->readInt32());
        uint64_t size = stream->readUint64();

        std::vector<uint8_t> data(size);
        stream->read(data.data(), data.size());
        variant.data = data;

        return variant;
    }

    DShader LoadDShader(const std::unique_ptr<Stream::DataReadStream> &stream) {
        DShader shader{};
        shader.name = stream->readString();
        uint64_t nVariants = stream->readUint64();
        for (uint64_t i = 0; i < nVariants; i++) {
            shader.variants.push_back(ReadDShaderVariant(stream));
        }
        return shader;
    }

    void WriteDShaderVariant(const std::unique_ptr<Stream::DataWriteStream> &stream, const ShaderVariant &variant) {
        stream->writeInt32(static_cast<int32_t>(variant.type));
        stream->writeInt32(static_cast<int32_t>(variant.api));
        stream->writeUint64(variant.data.size());
        stream->writeBuffer(variant.data.data(), variant.data.size());
    }

    void WriteDShader(const std::unique_ptr<Stream::DataWriteStream> &stream, const DShader &shader) {
        stream->writeString(shader.name);
        stream->writeUint64(shader.variants.size());
        for (const auto &variant : shader.variants) {
            WriteDShaderVariant(stream, variant);
        }
    }

    std::optional<ShaderVariant> DShader::find(ShaderType type, ShaderApi api) const {
        for (const auto &variant : variants) {
            if (variant.type == type && variant.api == api) {
                return variant;
            }
        }
        return std::nullopt;
    }

}