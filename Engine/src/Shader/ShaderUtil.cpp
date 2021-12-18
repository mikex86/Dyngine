#include "ShaderUtil.hpp"
#include <ErrorHandling/IllegalStateException.hpp>

namespace ShaderUtil {

    EXCEPTION_TYPE_DEFAULT_IMPL(ShaderLoadingException);
    EXCEPTION_TYPE_DEFAULT_IMPL(ShaderProgramCreationException);

    bool IsSupported(const std::shared_ptr<LLGL::RenderSystem> &renderSystem, LLGL::ShadingLanguage lang) {
        const auto &supportedShadingLanguages = renderSystem->GetRenderingCaps().shadingLanguages;
        return (std::find(supportedShadingLanguages.begin(), supportedShadingLanguages.end(), lang) !=
                supportedShadingLanguages.end());
    }

    LLGL::Shader *
    LoadSpirVShader(Dpac::ReadOnlyArchive &archive, const std::string &entryName,
                    const std::shared_ptr<LLGL::RenderSystem> &renderSystem,
                    LLGL::ShaderType shaderType,
                    const LLGL::VertexFormat &vertexFormat
    ) {
        if (!IsSupported(renderSystem, LLGL::ShadingLanguage::SPIRV)) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "Spir-V shading language not available");
        }

        uint8_t *shaderContent;
        uint64_t shaderContentSize;
        {
            auto shaderContentStream = archive.getEntryStream(entryName);
            shaderContentSize = archive.getUncompressedEntrySize(entryName);
            shaderContent = new uint8_t[shaderContentSize];
            shaderContentStream->read(shaderContent, shaderContentSize);
        }
        LLGL::ShaderDescriptor shaderDesc = {
                shaderType,
                reinterpret_cast<char *>(shaderContent),
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
            RAISE_EXCEPTION(ShaderLoadingException,
                            "Could not create shader from shader description. Shader Log: " + shaderLog);
        }
        return shader;
    }

    LLGL::Shader *LoadHLSLShader(Dpac::ReadOnlyArchive &archive, const std::string &entryName,
                                 const std::shared_ptr<LLGL::RenderSystem> &renderSystem, LLGL::ShaderType shaderType,
                                 const LLGL::VertexFormat &vertexFormat,
                                 const std::string &entryPoint,
                                 const std::string &profile) {

        if (!IsSupported(renderSystem, LLGL::ShadingLanguage::HLSL)) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "HLSL shading language not available");
        }

        uint8_t *shaderContent;
        uint64_t shaderContentSize;
        {
            auto shaderContentStream = archive.getEntryStream(entryName);
            shaderContentSize = archive.getUncompressedEntrySize(entryName);
            shaderContent = new uint8_t[shaderContentSize];
            shaderContentStream->read(shaderContent, shaderContentSize);
        }
        LLGL::ShaderDescriptor shaderDesc = {
                shaderType,
                reinterpret_cast<char *>(shaderContent),
                entryPoint.c_str(),
                profile.c_str()
        };
        shaderDesc.sourceType = LLGL::ShaderSourceType::BinaryBuffer;
        shaderDesc.sourceSize = shaderContentSize;
        shaderDesc.vertex.inputAttribs = vertexFormat.attributes;
        LLGL::Shader *shader = renderSystem->CreateShader(shaderDesc);
        delete[] shaderContent;

        if (shader == nullptr || shader->HasErrors()) {
            std::string shaderLog = shader->GetReport();
            RAISE_EXCEPTION(ShaderLoadingException,
                            "Could not create shader from shader description. Shader Log: " + shaderLog);
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

    LLGL::Shader *LoadGLSLShader(Dpac::ReadOnlyArchive &archive, const std::string &entryName,
                                 const std::shared_ptr<LLGL::RenderSystem> &renderSystem, LLGL::ShaderType shaderType,
                                 const LLGL::VertexFormat &vertexFormat) {
        if (!IsSupported(renderSystem, LLGL::ShadingLanguage::GLSL)) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "GLSL shading language not available");
        }

        uint8_t *shaderContent;
        uint64_t shaderContentSize;
        {
            auto shaderContentStream = archive.getEntryStream(entryName);
            shaderContentSize = archive.getUncompressedEntrySize(entryName);
            shaderContent = new uint8_t[shaderContentSize + 1];
            shaderContentStream->read(shaderContent, shaderContentSize);
            shaderContent[shaderContentSize] = '\0';
        }
        LLGL::ShaderDescriptor shaderDesc = {
                shaderType,
                reinterpret_cast<char *>(shaderContent),
                "main",
                nullptr
        };
        shaderDesc.sourceType = LLGL::ShaderSourceType::CodeString;
        shaderDesc.sourceSize = shaderContentSize;
        shaderDesc.vertex.inputAttribs = vertexFormat.attributes;

        LLGL::Shader *shader = renderSystem->CreateShader(shaderDesc);
        if (shader == nullptr || shader->HasErrors()) {
            std::string shaderLog = shader->GetReport();
            delete[] shaderContent;
            RAISE_EXCEPTION(ShaderLoadingException,
                            "Could not create shader from shader description. Shader Log: " + shaderLog);
        }
        return shader;
    }
}