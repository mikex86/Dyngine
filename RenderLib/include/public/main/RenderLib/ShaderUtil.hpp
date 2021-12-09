#pragma once

#include <RenderLib/ContextManagement.hpp>
#include <map>

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

    Shader *
    LoadPrecompiledShader(RenderLib::RenderContext *renderContext, const uint8_t *shaderBinaryContent,
                          size_t shaderBinaryContentSize, ShaderType shaderType);

    ShaderProgram *
    CreateShaderProgram(RenderLib::RenderContext *renderContext, const std::map<ShaderType, Shader *> &shaders);

}