#pragma once

#include <Dpac/Dpac.hpp>
#include <LLGL/LLGL.h>

namespace ShaderUtil {

    NEW_EXCEPTION_TYPE(ShaderLoadingException);

    NEW_EXCEPTION_TYPE(ShaderProgramCreationException);

    bool IsSupported(
            const LLGL::RenderSystem &renderSystem,
            LLGL::ShadingLanguage lang
    );

    LLGL::ShaderProgram *
    LoadDShaderPackage(LLGL::RenderSystem &renderSystem,
                       const std::unique_ptr<Stream::DataReadStream> &dShaderPackageStream,
                       const std::vector<LLGL::VertexAttribute> &vertexInputAttributes,
                       const std::vector<LLGL::FragmentAttribute> &fragmentOutputAttributes);

    LLGL::Shader *
    LoadSpirVShader(const std::unique_ptr<Stream::DataReadStream> &shaderContentStream,
                    LLGL::RenderSystem &renderSystem,
                    LLGL::ShaderType shaderType,
                    const std::vector<LLGL::VertexAttribute> &vertexInputAttributes,
                    const std::vector<LLGL::FragmentAttribute> &fragmentOutputAttributes);

    LLGL::Shader *
    LoadGLSLShader(const std::shared_ptr<Stream::DataReadStream> &shaderContentStream,
                   LLGL::RenderSystem &renderSystem,
                   LLGL::ShaderType shaderType,
                   const std::vector<LLGL::VertexAttribute> &vertexInputAttributes,
                   const std::vector<LLGL::FragmentAttribute> &fragmentOutputAttributes);

    LLGL::Shader *
    LoadHLSLShader(const std::unique_ptr<Stream::DataReadStream> &shaderContentStream,
                   LLGL::RenderSystem &renderSystem,
                   LLGL::ShaderType shaderType,
                   const std::vector<LLGL::VertexAttribute> &vertexInputAttributes,
                   const std::vector<LLGL::FragmentAttribute> &fragmentOutputAttributes,
                   const std::string &entryPoint,
                   const std::string &profile);


    LLGL::ShaderProgram *CreateShaderProgram(
            LLGL::RenderSystem &renderSystem,
            LLGL::Shader *vertexShader,
            LLGL::Shader *fragmentShader
    );

}