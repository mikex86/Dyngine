#include <RenderLib/CommandBufferVulkan_Internal.hpp>
#include <RenderLib/FrameBufferVulkan_Internal.hpp>
#include <memory>

namespace RenderLib {


    std::shared_ptr<RenderLib::CommandBuffer>
    CreateCommandBuffer(const std::shared_ptr<RenderLib::FrameBuffer> &framebuffer) {
        ENSURE_VULKAN_BACKEND_PTR(framebuffer);
        auto vulkanFrameBuffer = std::dynamic_pointer_cast<RenderLib::VulkanFrameBuffer>(framebuffer);
        auto vulkanRenderContext = vulkanFrameBuffer->vulkanRenderContext;
        auto swapChainFrameBuffers = vulkanFrameBuffer->frameBuffers;
        auto nCommandBuffers = static_cast<uint32_t>(swapChainFrameBuffers.size());

        std::vector<VkCommandBuffer> commandBuffers(swapChainFrameBuffers.size());

        {
            VkCommandBufferAllocateInfo allocInfo{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                    .commandPool = vulkanRenderContext->vkCommandPool,
                    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                    .commandBufferCount = nCommandBuffers
            };
            VULKAN_STATUS_VALIDATE(
                    vkAllocateCommandBuffers(vulkanRenderContext->vkDevice, &allocInfo, commandBuffers.data()),
                    "Failed to allocate command buffers!"
            );
        }

        auto commandBuffer = std::make_shared<RenderLib::VulkanCommandBuffer>(
                vulkanFrameBuffer, commandBuffers
        );

        return commandBuffer;
    }

    VulkanCommandBuffer::VulkanCommandBuffer(std::shared_ptr<VulkanFrameBuffer> vulkanFrameBuffer,
                                             std::vector<VkCommandBuffer> commandBuffers) : CommandBuffer(VULKAN),
                                                                                            vulkanFrameBuffer(std::move(
                                                                                                    vulkanFrameBuffer)),
                                                                                            commandBuffers(std::move(
                                                                                                    commandBuffers)) {
        changedSinceLastSubmit.resize(this->commandBuffers.size());
    }

    void VulkanCommandBuffer::setHasChanged(uint32_t commandBufferIndex) {
        changedSinceLastSubmit[commandBufferIndex] = true;
    }

    void VulkanCommandBuffer::clearChangedStatus(uint32_t commandBufferIndex) {
        changedSinceLastSubmit[commandBufferIndex] = false;
    }

    void VulkanCommandBuffer::begin() {
        VkCommandBuffer currentCommandBuffer = commandBuffers[vulkanFrameBuffer->vulkanRenderContext->currentImageIndex];
        vkResetCommandBuffer(currentCommandBuffer, 0);
        VkCommandBufferBeginInfo beginInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
                .pInheritanceInfo = nullptr
        };
        VULKAN_STATUS_VALIDATE(
                vkBeginCommandBuffer(currentCommandBuffer, &beginInfo),
                "Failed to begin recording command buffer!"
        );
        setHasChanged(vulkanFrameBuffer->vulkanRenderContext->currentImageIndex);
    }

    void VulkanCommandBuffer::end() {
        VkCommandBuffer currentCommandBuffer = commandBuffers[vulkanFrameBuffer->vulkanRenderContext->currentImageIndex];
        VULKAN_STATUS_VALIDATE(
                vkEndCommandBuffer(currentCommandBuffer),
                "Failed to end recording command buffer!"
        );
    }

    void VulkanCommandBuffer::beginRenderPass() {
        VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
        VkRenderPassBeginInfo beginRenderPassInfo{
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .renderPass = vulkanFrameBuffer->vulkanRenderContext->vkRenderPass,
                .framebuffer = vulkanFrameBuffer->frameBuffers[vulkanFrameBuffer->vulkanRenderContext->currentImageIndex],
                .renderArea = {
                        .offset = {0, 0},
                        .extent = vulkanFrameBuffer->vulkanRenderContext->vulkanSwapChain->swapChainExtent
                },
                .clearValueCount = 1,
                .pClearValues = &clearColor
        };
        vkCmdBeginRenderPass(commandBuffers[vulkanFrameBuffer->vulkanRenderContext->currentImageIndex],
                             &beginRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VulkanCommandBuffer::endRenderPass() {
        VkCommandBuffer currentCommandBuffer = commandBuffers[vulkanFrameBuffer->vulkanRenderContext->currentImageIndex];
        vkCmdEndRenderPass(currentCommandBuffer);
    }

    void VulkanCommandBuffer::bindGraphicsPipeline(const std::shared_ptr<GraphicsPipeline> &graphicsPipeline) {
        auto vulkanGraphicsPipeline = std::dynamic_pointer_cast<VulkanGraphicsPipeline>(graphicsPipeline);
        vkCmdBindPipeline(commandBuffers[vulkanFrameBuffer->vulkanRenderContext->currentImageIndex],
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          vulkanGraphicsPipeline->vkPipeline);
    }

    bool VulkanCommandBuffer::hasCommandBufferChanged(uint32_t commandBufferIndex) {
        return changedSinceLastSubmit[commandBufferIndex];
    }

}