#include <RenderLib/ShaderUtil.hpp>

namespace RenderLib {

    std::string GetShaderTypeName(ShaderType type) {
        switch (type) {
            case ShaderType::VERTEX_SHADER:
                return "VERTEX_SHADER";
            case ShaderType::FRAGMENT_SHADER:
                return "FRAGMENT_SHADER";
            case ShaderType::GEOMETRY_SHADER:
                return "GEOMETRY_SHADER";
            case ShaderType::COMPUTE_SHADER:
                return "COMPUTE_SHADER";
            default:
                return "UNKNOWN_SHADER_TYPE";
        }
    }

    ShaderProgram::ShaderProgram(RenderSystemBackend backend) : backend(backend) {
    }
}