#pragma once

#include <Stream/DataReadStream.hpp>
#include <Stream/DataWriteStream.hpp>
#include <vector>
#include <memory>
#include <optional>

namespace DShader {

    enum ShaderType {
        VERTEX_SHADER,
        FRAGMENT_SHADER
    };

    enum ShaderApi {
        GLSL_SOURCE,
        SPIRV_BINARY,
        HLSL_SOURCE,
        CSO_BINARY
    };

    struct ShaderVariant {
        ShaderType type;
        ShaderApi api;
        std::vector<uint8_t> data;
    };

    class DShader {

    public:
        std::string name;
        std::vector<ShaderVariant> variants;

        std::optional<ShaderVariant> find(ShaderType type, ShaderApi api) const;
    };

    DShader LoadDShader(const std::unique_ptr<Stream::DataReadStream> &stream);

    void WriteDShader(const std::unique_ptr<Stream::DataWriteStream> &stream, const DShader &shader);

}