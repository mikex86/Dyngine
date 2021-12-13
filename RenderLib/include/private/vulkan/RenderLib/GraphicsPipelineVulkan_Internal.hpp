#pragma once

#include <RenderLib/RenderLibVulkan_Internal.hpp>
#include <RenderLib/GraphicsPipeline.hpp>
#include <RenderLib/RenderContextVulkan_Internal.hpp>
#include <RenderLib/ShaderUtilVulkan_Internal.hpp>
#include <utility>
#include <vulkan/vulkan.h>

namespace RenderLib {

    struct VulkanGraphicsPipeline : public GraphicsPipeline {
    private:
        bool disposed = false;

    public:
        std::shared_ptr<VulkanRenderContext> vulkanRenderContext;
        VkPipeline vkPipeline;
        VkPipelineLayout vkPipelineLayout;

        VulkanGraphicsPipeline(const VertexFormat &vertexFormat, const PipelineLayout &pipelineLayout,
                               std::shared_ptr<ShaderProgram> shaderProgram,
                               std::shared_ptr<VulkanRenderContext> vulkanRenderContext,
                               VkPipeline vkPipeline, VkPipelineLayout vkPipelineLayout);

        void transferStateFrom(VulkanGraphicsPipeline *graphicsPipeline);

        void dispose();

        ~VulkanGraphicsPipeline() override;
    };

    VulkanGraphicsPipeline *
    CreateVulkanGraphicsPipelinePtr(const std::shared_ptr<RenderLib::RenderContext> &renderContext,
                                    const VertexFormat &vertexFormat,
                                    const PipelineLayout &pipelineLayout,
                                    const std::shared_ptr<RenderLib::ShaderProgram> &shaderProgram);
}
