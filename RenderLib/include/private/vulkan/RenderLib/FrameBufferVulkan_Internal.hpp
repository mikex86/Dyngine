#pragma once

#include <RenderLib/FrameBuffer.hpp>
#include <RenderLib/GraphicsPipelineVulkan_Internal.hpp>
#include <vulkan/vulkan.h>
#include <utility>
#include <vector>

namespace RenderLib {

    struct VulkanFrameBuffer : public FrameBuffer {
    public:
        std::shared_ptr<VulkanRenderContext> vulkanRenderContext;
        std::vector<VkFramebuffer> frameBuffers;

        VulkanFrameBuffer(std::shared_ptr<VulkanRenderContext> renderContext,
                          std::vector<VkFramebuffer> frameBuffers);

        ~VulkanFrameBuffer() override {
            for (auto frameBuffer : frameBuffers) {
                vkDestroyFramebuffer(vulkanRenderContext->vkDevice, frameBuffer, nullptr);
            }
            vulkanRenderContext = nullptr;
        }
    };

}