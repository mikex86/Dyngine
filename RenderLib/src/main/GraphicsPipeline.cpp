#include <RenderLib/GraphicsPipeline.hpp>
#include <utility>

namespace RenderLib {

    GraphicsPipeline::~GraphicsPipeline() = default;

    GraphicsPipeline::GraphicsPipeline(RenderSystemBackend backend, VertexFormat vertexFormat,
                                       PipelineLayout pipelineLayout, std::shared_ptr<ShaderProgram> shaderProgram)
            : backend(backend), vertexFormat(std::move(vertexFormat)), pipelineLayout(std::move(pipelineLayout)),
              shaderProgram(std::move(shaderProgram)) {
    }
}