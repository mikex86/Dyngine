#pragma once

#include <RenderLib/GraphicsPipeline.hpp>
#include <RenderLib/ContextManagementVulkan_Internal.hpp>
#include <RenderLib/ShaderUtilVulkan_Internal.hpp>
#include <vulkan/vulkan.h>

namespace RenderLib {

    struct VulkanGraphicsPipeline : public GraphicsPipeline {
    public:
        std::shared_ptr<VulkanRenderContext> vulkanRenderContext;
        VkPipeline vkPipeline;
        VkPipelineLayout vkPipelineLayout;
        VkRenderPass vkRenderPass;

        virtual ~VulkanGraphicsPipeline() {
            vkDestroyPipeline(vulkanRenderContext->vkDevice, vkPipeline, nullptr);
            vkDestroyPipelineLayout(vulkanRenderContext->vkDevice, vkPipelineLayout, nullptr);
            vkDestroyRenderPass(vulkanRenderContext->vkDevice, vkRenderPass, nullptr);
            vulkanRenderContext = nullptr;
        }
    };

}
