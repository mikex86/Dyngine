#pragma once

#include <RenderLib/RenderLibVulkan_Internal.hpp>
#include <RenderLib/CommandBuffer.hpp>
#include <RenderLib/RenderContextVulkan_Internal.hpp>
#include <utility>
#include <vulkan/vulkan.h>

namespace RenderLib {

    struct VulkanCommandBuffer : public CommandBuffer {
    private:
        bool disposed = false;
        std::vector<bool> changedSinceLastSubmit{};

    public:
        std::shared_ptr<VulkanRenderContext> vulkanRenderContext;
        std::vector<VkCommandBuffer> commandBuffers;

        VulkanCommandBuffer(std::shared_ptr<VulkanRenderContext> vulkanRenderContext,
                            std::vector<VkCommandBuffer> commandBuffers);

        ~VulkanCommandBuffer() override;

        void dispose();

        void begin() override;

        void beginRenderPass() override;

        void end() override;

        void endRenderPass() override;

        void bindGraphicsPipeline(const std::shared_ptr<RenderLib::GraphicsPipeline> &graphicsPipeline) override;

        void bindBufferObject(const std::shared_ptr<RenderLib::BufferObject> &bufferObject) override;

        void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;

        void transferStateFrom(VulkanCommandBuffer *commandBuffer);

    private:
        void setHasChanged(uint32_t commandBufferIndex);

    public:
        void clearChangedStatus(uint32_t commandBufferIndex);

        bool hasCommandBufferChanged(uint32_t commandBufferIndex);
    };

    VulkanCommandBuffer *CreateVulkanCommandBufferPtr(const std::shared_ptr<RenderContext> &renderContext);

}