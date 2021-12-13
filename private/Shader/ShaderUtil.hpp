#pragma once

#include <RenderLib/ShaderUtil.hpp>
#include <Dpac/Dpac.hpp>
#include <memory>

std::shared_ptr<RenderLib::Shader> LoadPrecompiledShaderFromArchive(const std::shared_ptr<RenderLib::RenderContext>& renderContext,
                                                    dpac::ReadOnlyArchive &archive,
                                                    const std::string &entryName,
                                                    RenderLib::ShaderType shaderType);