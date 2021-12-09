#pragma once

#include <RenderLib/ShaderUtil.hpp>
#include <Dpac/Dpac.hpp>

RenderLib::Shader *LoadPrecompiledShaderFromArchive(RenderLib::RenderContext *renderContext,
                                                   dpac::ReadOnlyArchive &archive,
                                                   const std::string &entryName,
                                                   RenderLib::ShaderType shaderType);