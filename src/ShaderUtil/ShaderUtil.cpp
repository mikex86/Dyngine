#include <Shader/ShaderUtil.hpp>

RenderLib::Shader *
LoadPrecompiledShaderFromArchive(RenderLib::RenderContext *renderContext, dpac::ReadOnlyArchive &archive,
                                 const std::string &entryName, RenderLib::ShaderType shaderType) {
    auto entryStream = archive.getEntryStream(entryName);
    auto streamSize = entryStream.getSize();
    auto shaderData = std::make_unique<uint8_t[]>(streamSize);
    for (size_t i = 0; i < streamSize; ++i) {
        shaderData[i] = entryStream.readUint8();
    }
    return RenderLib::LoadPrecompiledShader(renderContext, reinterpret_cast<const uint8_t *>(shaderData.get()), streamSize, shaderType);
}
