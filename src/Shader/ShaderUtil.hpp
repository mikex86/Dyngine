#pragma once

#include <Dpac/Dpac.hpp>
#include <LLGL/LLGL.h>

namespace ShaderUtil {

    NEW_EXCEPTION_TYPE(ShaderLoadingException);

    NEW_EXCEPTION_TYPE(ShaderProgramCreationException);

    bool IsSupported(
            const std::shared_ptr<LLGL::RenderSystem> &renderSystem,
            LLGL::ShadingLanguage lang
    );

    LLGL::Shader *
    LoadSpirVShader(dpac::ReadOnlyArchive &archive, const std::string &entryName,
                    const std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                    LLGL::ShaderType shaderType,
                    const LLGL::VertexFormat &vertexFormat);

    LLGL::Shader *
    LoadHLSLShader(dpac::ReadOnlyArchive &archive, const std::string &entryName,
                    const std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                    LLGL::ShaderType shaderType,
                    const LLGL::VertexFormat &vertexFormat,
                    const std::string &entryPoint,
                    const std::string &profile);


    LLGL::ShaderProgram *CreateShaderProgram(
            const std::shared_ptr<LLGL::RenderSystem> &renderSystem,
            LLGL::Shader *vertexShader,
            LLGL::Shader *fragmentShader
    );

}