#include "ShaderUtil.hpp"
#include <ErrorHandling/IllegalStateException.hpp>
#include <Dyngine_internal.hpp>

namespace ShaderUtil {

    EXCEPTION_TYPE_DEFAULT_IMPL(ShaderLoadingException);
    EXCEPTION_TYPE_DEFAULT_IMPL(ShaderProgramCreationException);


    bool IsSupported(const std::shared_ptr<LLGL::RenderSystem> &renderSystem, LLGL::ShadingLanguage lang) {
        const auto &supportedShadingLanguages = renderSystem->GetRenderingCaps().shadingLanguages;
        return (std::find(supportedShadingLanguages.begin(), supportedShadingLanguages.end(), lang) != supportedShadingLanguages.end());
    }

    LLGL::Shader *
    LoadSpirVShader(dpac::ReadOnlyArchive &archive, const std::string &entryName,
                    const std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                    LLGL::ShaderType shaderType,
                    const LLGL::VertexFormat &vertexFormat
    ) {
        if (!IsSupported(renderSystem, LLGL::ShadingLanguage::SPIRV)) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "Spir-V shading language not available");
        }

        char *shaderContent;
        uint64_t shaderContentSize;
        {
            auto shaderContentStream = archive.getEntryStream(entryName);
            shaderContentSize = shaderContentStream.getSize();
            shaderContent = new char[shaderContentSize];

            for (uint64_t i = 0; i < shaderContentSize; i++) {
                char byte = shaderContentStream.readInt8();
                shaderContent[i] = byte;
            }
        }
        LLGL::ShaderDescriptor shaderDesc = {
                shaderType,
                shaderContent,
                "main",
                nullptr
        };
        shaderDesc.sourceType = LLGL::ShaderSourceType::BinaryBuffer;
        shaderDesc.sourceSize = shaderContentSize;
        shaderDesc.vertex.inputAttribs = vertexFormat.attributes;

        LLGL::Shader *shader = renderSystem->CreateShader(shaderDesc);
        delete[] shaderContent;
        if (shader == nullptr || shader->HasErrors()) {
            std::string shaderLog = shader->GetReport();
            RAISE_EXCEPTION(ShaderLoadingException, "Could not create shader from shader description. Shader Log: " + shaderLog);
        }
        return shader;
    }

    LLGL::Shader *LoadHLSLShader(dpac::ReadOnlyArchive &archive, const std::string &entryName,
                                 const std::shared_ptr<LLGL::RenderSystem> &renderSystem, LLGL::ShaderType shaderType,
                                 const LLGL::VertexFormat &vertexFormat,
                                 const std::string &entryPoint,
                                 const std::string &profile) {

        if (!IsSupported(renderSystem, LLGL::ShadingLanguage::HLSL)) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "HLSL shading language not available");
        }

        char *shaderContent;
        uint64_t shaderContentSize;
        {
            auto shaderContentStream = archive.getEntryStream(entryName);
            shaderContentSize = shaderContentStream.getSize();
            shaderContent = new char[shaderContentSize];

            for (uint64_t i = 0; i < shaderContentSize; i++) {
                char byte = shaderContentStream.readInt8();
                shaderContent[i] = byte;
            }
        }
        LLGL::ShaderDescriptor shaderDesc = {
                shaderType,
                shaderContent,
                entryPoint.c_str(),
                profile.c_str()
        };
        shaderDesc.sourceType = LLGL::ShaderSourceType::BinaryBuffer;
        shaderDesc.source = shaderContent;
        shaderDesc.sourceSize = shaderContentSize;
        shaderDesc.vertex.inputAttribs = vertexFormat.attributes;
        LLGL::Shader *shader = renderSystem->CreateShader(shaderDesc);
        delete[] shaderContent;

        if (shader == nullptr || shader->HasErrors()) {
            std::string shaderLog = shader->GetReport();
            RAISE_EXCEPTION(ShaderLoadingException, "Could not create shader from shader description. Shader Log: " + shaderLog);
        }
        return shader;
    }

    LLGL::ShaderProgram *
    CreateShaderProgram(
            const std::shared_ptr<LLGL::RenderSystem> &renderSystem,
            LLGL::Shader *vertexShader,
            LLGL::Shader *fragmentShader) {
        LLGL::ShaderProgramDescriptor shaderProgramDesc;
        {
            shaderProgramDesc.vertexShader = vertexShader;
            shaderProgramDesc.fragmentShader = fragmentShader;
        }
        LLGL::ShaderProgram *shaderProgram = renderSystem->CreateShaderProgram(shaderProgramDesc);
        if (shaderProgram == nullptr || shaderProgram->HasErrors()) {
            RAISE_EXCEPTION(ShaderProgramCreationException, "Shader program has errors: " + shaderProgram->GetReport());
        }
        return shaderProgram;
    }
}