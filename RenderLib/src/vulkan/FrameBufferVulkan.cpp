#include <RenderLib/FrameBufferVulkan_Internal.hpp>
#include <memory>

namespace RenderLib {


    static std::vector<VkFramebuffer>
    CreateFrameBuffers(const VkDevice &vkDevice,
                       const VkRenderPass &vkRenderPass,
                       const std::vector<VkImageView> &swapChainImageViews,
                       VkExtent2D swapChainExtent) {
        std::vector<VkFramebuffer> swapChainFrameBuffers(swapChainImageViews.size());
        for (uint32_t i = 0; i < swapChainImageViews.size(); i++) {
            VkImageView attachments[] = {
                    swapChainImageViews[i]
            };
            VkFramebufferCreateInfo framebufferInfo{
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .renderPass = vkRenderPass,
                    .attachmentCount = 1,
                    .pAttachments = attachments,
                    .width = swapChainExtent.width,
                    .height = swapChainExtent.height,
                    .layers = 1
            };
            VULKAN_STATUS_VALIDATE(
                    vkCreateFramebuffer(vkDevice, &framebufferInfo, nullptr, &swapChainFrameBuffers[i]),
                    "Failed to create framebuffer"
            );
        }
        return swapChainFrameBuffers;
    }

    std::shared_ptr<FrameBuffer> CreateFrameBuffer(const std::shared_ptr<RenderLib::RenderContext> &renderContext) {
        ENSURE_VULKAN_BACKEND_PTR(renderContext);
        auto vulkanRenderContext = std::dynamic_pointer_cast<RenderLib::VulkanRenderContext>(
                renderContext);
        auto vulkanDevice = vulkanRenderContext->vkDevice;
        auto renderPass = vulkanRenderContext->vkRenderPass;
        auto swapChain = vulkanRenderContext->vulkanSwapChain;
        auto swapChainImageViews = swapChain->swapChainImageViews;
        auto swapChainExtent = swapChain->swapChainExtent;
        auto frameBuffers = CreateFrameBuffers(vulkanDevice,
                                               renderPass,
                                               swapChainImageViews,
                                               swapChainExtent);
        return std::make_shared<VulkanFrameBuffer>(vulkanRenderContext, frameBuffers);
    }


    VulkanFrameBuffer::VulkanFrameBuffer(std::shared_ptr<VulkanRenderContext> renderContext,
                                         std::vector<VkFramebuffer> frameBuffers) : FrameBuffer(VULKAN),
                                                                                    vulkanRenderContext(
                                                                                            std::move(renderContext)),
                                                                                    frameBuffers(
                                                                                            std::move(frameBuffers)) {
    }
}