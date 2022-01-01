#include "Dyngine/Rendering/Shader/ShaderUtil.hpp"

#include "Stream/MemoryDataStream.hpp"
#include "ErrorHandling/IllegalStateException.hpp"
#include "DShader/DShader.hpp"

namespace ShaderUtil {

    EXCEPTION_TYPE_DEFAULT_IMPL(ShaderLoadingException);
    EXCEPTION_TYPE_DEFAULT_IMPL(ShaderProgramCreationException);

    bool IsSupported(const LLGL::RenderSystem &renderSystem, LLGL::ShadingLanguage lang) {
        const auto &supportedShadingLanguages = renderSystem.GetRenderingCaps().shadingLanguages;
        return (std::find(supportedShadingLanguages.begin(), supportedShadingLanguages.end(), lang) !=
                supportedShadingLanguages.end());
    }

    LLGL::Shader *
    LoadSpirVShader(const std::unique_ptr<Stream::DataReadStream> &stream,
                    LLGL::RenderSystem &renderSystem,
                    LLGL::ShaderType shaderType,
                    const std::vector<LLGL::VertexAttribute> &vertexInputAttributes,
                    const std::vector<LLGL::FragmentAttribute> &fragmentOutputAttributes
    ) {
        if (!IsSupported(renderSystem, LLGL::ShadingLanguage::SPIRV)) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "Spir-V shading language not available");
        }

        uint8_t *shaderContent;
        uint64_t shaderContentSize;
        {
            shaderContentSize = stream->getLength();
            shaderContent = new uint8_t[shaderContentSize];
            stream->read(shaderContent, shaderContentSize);
        }
        LLGL::ShaderDescriptor shaderDesc = {
                shaderType,
                reinterpret_cast<char *>(shaderContent),
                "main",
                nullptr
        };
        shaderDesc.sourceType = LLGL::ShaderSourceType::BinaryBuffer;
        shaderDesc.sourceSize = shaderContentSize;
        shaderDesc.vertex.inputAttribs = vertexInputAttributes;
        shaderDesc.fragment.outputAttribs = fragmentOutputAttributes;

        LLGL::Shader *shader = renderSystem.CreateShader(shaderDesc);
        delete[] shaderContent;
        if (shader == nullptr || shader->HasErrors()) {
            std::string shaderLog = shader->GetReport();
            RAISE_EXCEPTION(ShaderLoadingException,
                            "Could not create shader from shader description. Shader Log: " + shaderLog);
        }
        return shader;
    }

    LLGL::Shader *LoadHLSLShader(const std::unique_ptr<Stream::DataReadStream> &shaderContentStream,
                                 LLGL::RenderSystem &renderSystem, LLGL::ShaderType shaderType,
                                 const std::vector<LLGL::VertexAttribute> &vertexInputAttributes,
                                 const std::vector<LLGL::FragmentAttribute> &fragmentOutputAttributes,
                                 const std::string &entryPoint,
                                 const std::string &profile) {

        if (!IsSupported(renderSystem, LLGL::ShadingLanguage::HLSL)) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "HLSL shading language not available");
        }

        uint8_t *shaderContent;
        uint64_t shaderContentSize;
        {
            shaderContentSize = shaderContentStream->getLength();
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
        shaderDesc.vertex.inputAttribs = vertexInputAttributes;
        shaderDesc.fragment.outputAttribs = fragmentOutputAttributes;
        LLGL::Shader *shader = renderSystem.CreateShader(shaderDesc);
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
            LLGL::RenderSystem &renderSystem,
            LLGL::Shader *vertexShader,
            LLGL::Shader *fragmentShader) {
        LLGL::ShaderProgramDescriptor shaderProgramDesc;
        {
            shaderProgramDesc.vertexShader = vertexShader;
            shaderProgramDesc.fragmentShader = fragmentShader;
        }
        LLGL::ShaderProgram *shaderProgram = renderSystem.CreateShaderProgram(shaderProgramDesc);
        if (shaderProgram == nullptr || shaderProgram->HasErrors()) {
            std::string shaderLog = shaderProgram->GetReport();
            std::cerr << "Could not create shader program. Shader Log: " << shaderLog << std::endl;
            RAISE_EXCEPTION(ShaderProgramCreationException, "Shader program has errors: " + shaderProgram->GetReport());
        }
        return shaderProgram;
    }

    LLGL::Shader *LoadGLSLShader(const std::unique_ptr<Stream::DataReadStream> &shaderContentStream,
                                 LLGL::RenderSystem &renderSystem, LLGL::ShaderType shaderType,
                                 const std::vector<LLGL::VertexAttribute> &vertexInputAttributes,
                                 const std::vector<LLGL::FragmentAttribute> &fragmentOutputAttributes) {
        if (!IsSupported(renderSystem, LLGL::ShadingLanguage::GLSL)) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "GLSL shading language not available");
        }

        uint8_t *shaderContent;
        uint64_t shaderContentSize;
        {
            shaderContentSize = shaderContentStream->getLength();
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
        shaderDesc.vertex.inputAttribs = vertexInputAttributes;
        shaderDesc.fragment.outputAttribs = fragmentOutputAttributes;

        LLGL::Shader *shader = renderSystem.CreateShader(shaderDesc);
        if (shader == nullptr || shader->HasErrors()) {
            std::string shaderLog = shader->GetReport();
            delete[] shaderContent;
            RAISE_EXCEPTION(ShaderLoadingException,
                            "Could not create shader from shader description. Shader Log: " + shaderLog);
        }
        return shader;
    }

    LLGL::ShaderProgram *LoadDShaderPackage(LLGL::RenderSystem &renderSystem,
                                            const std::unique_ptr<Stream::DataReadStream> &dShaderPackageStream,
                                            const std::vector<LLGL::VertexAttribute> &vertexInputAttributes,
                                            const std::vector<LLGL::FragmentAttribute> &fragmentOutputAttributes) {
        auto dShaderPackage = DShader::LoadDShader(dShaderPackageStream);
        std::vector<uint8_t> vertexShaderContent{}, fragmentShaderContent{};
        if (IsSupported(renderSystem, LLGL::ShadingLanguage::SPIRV)) {
            // vertex shader
            {
                auto vertSpirVOpt = dShaderPackage.find(DShader::ShaderType::VERTEX_SHADER,
                                                        DShader::ShaderApi::SPIRV_BINARY);
                if (vertSpirVOpt) {
                    vertexShaderContent = vertSpirVOpt.value().data;
                }
            }
            // fragment shader
            {
                auto fragSpirVOpt = dShaderPackage.find(DShader::ShaderType::FRAGMENT_SHADER,
                                                        DShader::ShaderApi::SPIRV_BINARY);
                if (fragSpirVOpt) {
                    fragmentShaderContent = fragSpirVOpt.value().data;
                }
            }
        } else if (IsSupported(renderSystem, LLGL::ShadingLanguage::HLSL)) {
            // vertex shader
            {
                auto vertHlslOpt = dShaderPackage.find(DShader::ShaderType::VERTEX_SHADER,
                                                       DShader::ShaderApi::CSO_BINARY);
                if (vertHlslOpt) {
                    vertexShaderContent = vertHlslOpt.value().data;
                } else {
                    vertHlslOpt = dShaderPackage.find(DShader::ShaderType::VERTEX_SHADER,
                                                      DShader::ShaderApi::HLSL_SOURCE);
                    if (vertHlslOpt) {
                        vertexShaderContent = vertHlslOpt.value().data;
                    }
                }
            }
            // fragment shader
            {
                // Compiled Shader Object (CSO) (precompiled HLSL)
                auto fragHlslOpt = dShaderPackage.find(DShader::ShaderType::FRAGMENT_SHADER,
                                                       DShader::ShaderApi::CSO_BINARY);
                if (fragHlslOpt) {
                    fragmentShaderContent = fragHlslOpt->data;
                } else {
                    // HLSL source (text base)
                    fragHlslOpt = dShaderPackage.find(DShader::ShaderType::FRAGMENT_SHADER,
                                                      DShader::ShaderApi::HLSL_SOURCE);
                    if (fragHlslOpt) {
                        fragmentShaderContent = fragHlslOpt->data;
                    }
                }
            }
        } else if (IsSupported(renderSystem, LLGL::ShadingLanguage::GLSL)) {
            // vertex shader
            {
                auto vertGLSLOpt = dShaderPackage.find(DShader::ShaderType::VERTEX_SHADER,
                                                       DShader::ShaderApi::GLSL_SOURCE);
                if (vertGLSLOpt) {
                    vertexShaderContent = vertGLSLOpt.value().data;
                }
            }
            // fragment shader
            {
                auto fragGLSLOpt = dShaderPackage.find(DShader::ShaderType::FRAGMENT_SHADER,
                                                       DShader::ShaderApi::GLSL_SOURCE);
                if (fragGLSLOpt) {
                    fragmentShaderContent = fragGLSLOpt.value().data;
                }
            }
        }
        if (vertexShaderContent.empty()) {
            throw std::runtime_error("Vertex shader not found in DShader package");
        }
        if (fragmentShaderContent.empty()) {
            throw std::runtime_error("Fragment shader not found in DShader package");
        }
        auto vertexShader = LoadSpirVShader(
                Stream::MemoryReadStream::CopyOf(vertexShaderContent.data(), vertexShaderContent.size()),
                renderSystem, LLGL::ShaderType::Vertex, vertexInputAttributes, fragmentOutputAttributes
        );
        auto fragmentShader = LoadSpirVShader(
                Stream::MemoryReadStream::CopyOf(fragmentShaderContent.data(), fragmentShaderContent.size()),
                renderSystem, LLGL::ShaderType::Fragment, vertexInputAttributes, fragmentOutputAttributes
        );
        LLGL::ShaderProgram *shaderProgram = CreateShaderProgram(renderSystem, vertexShader, fragmentShader);
        renderSystem.Release(*vertexShader);
        renderSystem.Release(*fragmentShader);
        return shaderProgram;
    }
}