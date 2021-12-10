#pragma once

#include <RenderLib/GraphicsPipeline.hpp>
#include <RenderLib/RenderContextVulkan_Internal.hpp>
#include <RenderLib/ShaderUtilVulkan_Internal.hpp>
#include <utility>
#include <vulkan/vulkan.h>

namespace RenderLib {

    struct VulkanGraphicsPipeline : public GraphicsPipeline {
    public:
        std::shared_ptr<VulkanRenderContext> vulkanRenderContext;
        VkPipeline vkPipeline;
        VkPipelineLayout vkPipelineLayout;

        VulkanGraphicsPipeline(std::shared_ptr<VulkanRenderContext> vulkanRenderContext,
                               VkPipeline vkPipeline, VkPipelineLayout vkPipelineLayout);

        ~VulkanGraphicsPipeline() override;
    };

}
