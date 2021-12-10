#include <RenderLib/GraphicsPipeline.hpp>

namespace RenderLib {

    GraphicsPipeline::GraphicsPipeline(RenderSystemBackend backend) : backend(backend) {
    }

    GraphicsPipeline::~GraphicsPipeline() = default;

}