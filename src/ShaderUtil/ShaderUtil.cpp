#include <Shader/ShaderUtil.hpp>
#include <Buffer/FileDataReadBuffer.hpp>
#include <iostream>

std::shared_ptr<RenderLib::Shader>
LoadPrecompiledShaderFromArchive(const std::shared_ptr<RenderLib::RenderContext> &renderContext,
                                 dpac::ReadOnlyArchive &archive,
                                 const std::string &entryName, RenderLib::ShaderType shaderType) {
    auto entryStream = archive.getEntryStream(entryName);
    auto streamSize = entryStream.getSize();
    auto shaderData = std::make_unique<uint8_t[]>(streamSize);
    for (size_t i = 0; i < streamSize; ++i) {
        shaderData[i] = entryStream.readUint8();
    }
    return RenderLib::LoadPrecompiledShader(renderContext, reinterpret_cast<const uint8_t *>(shaderData.get()),
                                            streamSize, shaderType);
}

std::shared_ptr<RenderLib::Shader>
LoadPrecompiledShaderFromFile(const std::shared_ptr<RenderLib::RenderContext> &renderContext,
                              const std::string &filePath,
                              RenderLib::ShaderType shaderType) {
    auto fileStream = buffer::FileDataReadBuffer::Open(filePath);
    auto streamSize = fileStream.getSize();
    auto shaderData = std::make_unique<uint8_t[]>(streamSize);
    for (size_t i = 0; i < streamSize; ++i) {
        shaderData[i] = fileStream.readUint8();
        std::cout << (int) shaderData[i] << " ";
    }
    std::cout << std::endl;
    return RenderLib::LoadPrecompiledShader(renderContext, reinterpret_cast<const uint8_t *>(shaderData.get()),
                                            streamSize, shaderType);
}
