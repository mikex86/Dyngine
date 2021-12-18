#include <DShader/DShader.hpp>
#include <ErrorHandling/IllegalStateException.hpp>
#include <Stream/FileDataWriteStream.hpp>
#include <iostream>
#include <filesystem>
#include <vector>
#include <fstream>

DShader::ShaderApi GetShaderApiFromExtension(const std::string &filePath);

DShader::ShaderVariant MakeShaderVariant(const std::string &filePath);

DShader::ShaderType GetShaderTypeFromExtension(const std::string &filePath);

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory_with_shader_variants> <dshader_output_file>" << std::endl;
        return 1;
    }
    std::string shaderVariantsDir = argv[1];
    std::string outputFile = argv[2];

    std::vector<DShader::ShaderVariant> shaderVariants;
    // iterate over all shader variant files in shaderVariantsDir
    // and create a DShaderPackage from them and write it to outputFile
    for (const auto &entry: std::filesystem::directory_iterator(shaderVariantsDir)) {
        std::string shaderVariantFile = entry.path().string();
        DShader::ShaderVariant shaderVariant = MakeShaderVariant(shaderVariantFile);
        shaderVariants.push_back(shaderVariant);
    }
    auto stream = Stream::FileDataWriteStream::Open(outputFile);
    DShader::DShader shader{
            .name = std::filesystem::path(outputFile).stem().string(),
            .variants = shaderVariants
    };
    DShader::WriteDShader(stream, shader);
}

DShader::ShaderVariant MakeShaderVariant(const std::string &filePath) {
    DShader::ShaderApi api = GetShaderApiFromExtension(filePath);
    DShader::ShaderType shaderType = GetShaderTypeFromExtension(filePath);
    std::vector<uint8_t> shaderData{};
    std::ifstream shaderFile(filePath, std::ifstream::binary);
    if (!shaderFile.is_open()) {
        RAISE_EXCEPTION(errorhandling::IllegalStateException, "Could not open shader file: " + filePath);
    }
    shaderFile.seekg(0, std::ios::end);
    shaderData.reserve(shaderFile.tellg());
    shaderFile.seekg(0, std::ios::beg);
    shaderData.insert(shaderData.begin(), std::istreambuf_iterator<char>(shaderFile), std::istreambuf_iterator<char>());
    shaderFile.close();
    return DShader::ShaderVariant{
            .type = shaderType,
            .api = api,
            .data = shaderData
    };
}


DShader::ShaderApi GetShaderApiFromExtension(const std::string &filePath) {
    std::string extension = filePath.substr(filePath.find_last_of('.') + 1);
    if (extension == "glsl") {
        return DShader::ShaderApi::GLSL_SOURCE;
    } else if (extension == "hlsl") {
        return DShader::ShaderApi::HLSL_SOURCE;
    } else if (extension == "spv") {
        return DShader::ShaderApi::SPIRV_BINARY;
    } else if (extension == "cso") {
        return DShader::ShaderApi::CSO_BINARY;
    }
    RAISE_EXCEPTION(errorhandling::IllegalStateException, "Unknown shader extension: " + extension);
}

DShader::ShaderType GetShaderTypeFromExtension(const std::string &filePath) {
    std::string fileRest = filePath;
    {
        auto index = fileRest.find(".spv");
        if (index != std::string::npos) {
            fileRest = fileRest.substr(0, index);
        }
        index = fileRest.find(".cso");
        if (index != std::string::npos) {
            fileRest = fileRest.substr(0, index);
        }
    }
    fileRest = fileRest.substr(0, fileRest.find_last_of('.'));
    std::string shaderTypeString = fileRest.substr(fileRest.find_last_of('.') + 1);
    if (shaderTypeString == "vert") {
        return DShader::ShaderType::VERTEX_SHADER;
    } else if (shaderTypeString == "frag") {
        return DShader::ShaderType::FRAGMENT_SHADER;
    }
    RAISE_EXCEPTION(errorhandling::IllegalStateException, "Unknown shader type: " + shaderTypeString);
}