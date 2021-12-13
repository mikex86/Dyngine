#include <RenderLib/CommandBufferVulkan_Internal.hpp>
#include <RenderLib/GraphicsPipelineVulkan_Internal.hpp>
#include <RenderLib/BufferObjectVulkan_Internal.hpp>
#include <memory>

namespace RenderLib {


    VulkanCommandBuffer *CreateVulkanCommandBufferPtr(const std::shared_ptr<RenderContext> &renderContext) {
        ENSURE_VULKAN_BACKEND_PTR(renderContext);
        auto vulkanRenderContext = std::dynamic_pointer_cast<RenderLib::VulkanRenderContext>(renderContext);
        auto vulkanFrameBuffer = vulkanRenderContext->vulkanFrameBuffer;
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

        auto commandBuffer = new RenderLib::VulkanCommandBuffer(
                vulkanRenderContext, commandBuffers
        );
        return commandBuffer;
    }

    std::shared_ptr<RenderLib::CommandBuffer>
    CreateCommandBuffer(const std::shared_ptr<RenderLib::RenderContext> &renderContext) {
        ENSURE_VULKAN_BACKEND_PTR(renderContext);
        auto vulkanRenderContext = std::dynamic_pointer_cast<RenderLib::VulkanRenderContext>(renderContext);
        auto commandBuffer = std::shared_ptr<VulkanCommandBuffer>(CreateVulkanCommandBufferPtr(renderContext));
        vulkanRenderContext->vulkanCommandBuffers.push_back(commandBuffer);
        return commandBuffer;
    }

    VulkanCommandBuffer::VulkanCommandBuffer(std::shared_ptr<VulkanRenderContext> vulkanRenderContext,
                                             std::vector<VkCommandBuffer> commandBuffers) : CommandBuffer(VULKAN),
                                                                                            vulkanRenderContext(std::move(vulkanRenderContext)),
                                                                                            commandBuffers(std::move(commandBuffers)) {
        changedSinceLastSubmit.resize(this->commandBuffers.size());
    }

    void VulkanCommandBuffer::setHasChanged(uint32_t commandBufferIndex) {
        changedSinceLastSubmit[commandBufferIndex] = true;
    }

    void VulkanCommandBuffer::clearChangedStatus(uint32_t commandBufferIndex) {
        changedSinceLastSubmit[commandBufferIndex] = false;
    }

    void VulkanCommandBuffer::begin() {
        VkCommandBuffer currentCommandBuffer = commandBuffers[vulkanRenderContext->currentImageIndex];
        vkResetCommandBuffer(currentCommandBuffer, 0); // TODO: SMARTER HANDLING FOR THIS CASE
        VkCommandBufferBeginInfo beginInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
                .pInheritanceInfo = nullptr
        };
        VULKAN_STATUS_VALIDATE(
                vkBeginCommandBuffer(currentCommandBuffer, &beginInfo),
                "Failed to begin recording command buffer!"
        );
        setHasChanged(vulkanRenderContext->currentImageIndex);
    }

    void VulkanCommandBuffer::end() {
        VkCommandBuffer currentCommandBuffer = commandBuffers[vulkanRenderContext->currentImageIndex];
        VULKAN_STATUS_VALIDATE(
                vkEndCommandBuffer(currentCommandBuffer),
                "Failed to end recording command buffer!"
        );
    }

    void VulkanCommandBuffer::beginRenderPass() {
        VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
        VkRenderPassBeginInfo beginRenderPassInfo{
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .renderPass = vulkanRenderContext->vkRenderPass,
                .framebuffer = vulkanRenderContext->vulkanFrameBuffer->frameBuffers[vulkanRenderContext->currentImageIndex],
                .renderArea = {
                        .offset = {0, 0},
                        .extent = vulkanRenderContext->vulkanSwapChain->swapChainExtent
                },
                .clearValueCount = 1,
                .pClearValues = &clearColor
        };
        vkCmdBeginRenderPass(commandBuffers[vulkanRenderContext->currentImageIndex],
                             &beginRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VulkanCommandBuffer::endRenderPass() {
        VkCommandBuffer currentCommandBuffer = commandBuffers[vulkanRenderContext->currentImageIndex];
        vkCmdEndRenderPass(currentCommandBuffer);
    }

    void VulkanCommandBuffer::bindGraphicsPipeline(const std::shared_ptr<GraphicsPipeline> &graphicsPipeline) {
        auto vulkanGraphicsPipeline = std::dynamic_pointer_cast<VulkanGraphicsPipeline>(graphicsPipeline);
        vkCmdBindPipeline(commandBuffers[vulkanRenderContext->currentImageIndex],
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          vulkanGraphicsPipeline->vkPipeline);
    }

    void VulkanCommandBuffer::bindBufferObject(const std::shared_ptr<RenderLib::BufferObject> &bufferObject) {
        ENSURE_VULKAN_BACKEND_PTR(bufferObject);
        auto vulkanBufferObject = std::dynamic_pointer_cast<VulkanBufferObject>(bufferObject);
        switch (bufferObject->bufferType) {
            case BufferType::VERTEX_BUFFER: {
                VkDeviceSize offset = 0;
                vkCmdBindVertexBuffers(
                        commandBuffers[vulkanRenderContext->currentImageIndex],
                        0,
                        1,
                        &vulkanBufferObject->vkBuffer,
                        &offset
                );
                break;
            }
            case BufferType::INDEX_BUFFER: {
                VkDeviceSize offset = 0;
                vkCmdBindIndexBuffer(
                        commandBuffers[vulkanRenderContext->currentImageIndex],
                        vulkanBufferObject->vkBuffer,
                        offset,
                        VK_INDEX_TYPE_UINT32 // TODO: FIX HARD CODING
                );
                break;
            }
        }
    }

    void VulkanCommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                                   uint32_t firstInstance) {
        vkCmdDraw(commandBuffers[vulkanRenderContext->currentImageIndex],
                  vertexCount,
                  instanceCount,
                  firstVertex,
                  firstInstance
        );
    }

    bool VulkanCommandBuffer::hasCommandBufferChanged(uint32_t commandBufferIndex) {
        return changedSinceLastSubmit[commandBufferIndex];
    }

    void VulkanCommandBuffer::dispose() {
        if (disposed) {
            return;
        }
        vkFreeCommandBuffers(vulkanRenderContext->vkDevice,
                             vulkanRenderContext->vkCommandPool,
                             static_cast<uint32_t>(commandBuffers.size()),
                             commandBuffers.data());
        disposed = true;
    }

    VulkanCommandBuffer::~VulkanCommandBuffer() {
        dispose();
    }

    void VulkanCommandBuffer::transferStateFrom(VulkanCommandBuffer *commandBuffer) {
        commandBuffers = commandBuffer->commandBuffers;
        vulkanRenderContext = commandBuffer->vulkanRenderContext;
        changedSinceLastSubmit = commandBuffer->changedSinceLastSubmit;
        commandBuffer->disposed = true; // Hack to prevent the handles we just transferred from being disposed
    }

}