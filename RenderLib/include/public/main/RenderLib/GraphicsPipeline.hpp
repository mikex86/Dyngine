#pragma once

#include <RenderLib/ShaderUtil.hpp>
#include <RenderLib/VertexFormat.hpp>
#include <memory>

namespace RenderLib {

    struct GraphicsPipeline {
    public:
        RenderSystemBackend backend;

        explicit GraphicsPipeline(RenderSystemBackend backend);

        virtual ~GraphicsPipeline();
    };

    /**
     * Describes a uniform variable that the pipeline's shader accepts.
     */
    struct UniformDescriptor {

    public:
        /**
         * The number of elements that the uniform data consists of.
         * If greater than 1, the uniform variable is an array.
         */
        uint32_t elementCount;

        /**
         * The shader types (stages) that accept the uniform variable.
         */
        std::vector<ShaderType> acceptingShaderTypes;
    };

    struct PipelineLayout {

        std::vector<UniformDescriptor> uniformDescriptors;

    };

    std::shared_ptr<GraphicsPipeline> CreateGraphicsPipeline(const std::shared_ptr<RenderLib::RenderContext> &renderContext,
                                                             const VertexFormat &vertexFormat,
                                                             const PipelineLayout &pipelineLayout,
                                                             const std::shared_ptr<RenderLib::ShaderProgram> &shaderProgram);

}