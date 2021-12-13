#pragma once

#include <RenderLib/RenderContext.hpp>
#include <map>
#include <memory>

namespace RenderLib {

    enum ShaderType {
        VERTEX_SHADER,
        FRAGMENT_SHADER,
        COMPUTE_SHADER,
        GEOMETRY_SHADER,
    };

    std::string GetShaderTypeName(ShaderType type);

    struct Shader {
    public:
        RenderSystemBackend backend;
        ShaderType shaderType;

        explicit Shader(RenderSystemBackend backend, ShaderType shaderType) : backend(backend),
                                                                              shaderType(shaderType) {}

        virtual ~Shader() = default;
    };

    struct ShaderProgram {

    public:
        RenderSystemBackend backend;

        explicit ShaderProgram(RenderSystemBackend backend);

        virtual ~ShaderProgram() = default;
    };

    std::shared_ptr<RenderLib::Shader>
    LoadPrecompiledShader(const std::shared_ptr<RenderLib::RenderContext> &renderContext, const uint8_t *shaderBinaryContent,
                          size_t shaderBinaryContentSize, ShaderType shaderType);

    std::shared_ptr<RenderLib::ShaderProgram>
    CreateShaderProgram(const std::shared_ptr<RenderLib::RenderContext> &renderContext, const std::map<ShaderType, Shader *> &shaders);

}