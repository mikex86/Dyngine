#pragma once

#include <RenderLib/CommandBuffer.hpp>
#include <RenderLib/RenderContextVulkan_Internal.hpp>
#include <RenderLib/FrameBufferVulkan_Internal.hpp>
#include <utility>
#include <vulkan/vulkan.h>

namespace RenderLib {

    struct VulkanCommandBuffer : public CommandBuffer {
    private:
        std::vector<bool> changedSinceLastSubmit{};

    public:
        std::shared_ptr<VulkanFrameBuffer> vulkanFrameBuffer;
        std::vector<VkCommandBuffer> commandBuffers;

        VulkanCommandBuffer(std::shared_ptr<VulkanFrameBuffer> vulkanFrameBuffer,
                            std::vector<VkCommandBuffer> commandBuffers);

        void begin() override;

        void beginRenderPass() override;

        void end() override;

        void endRenderPass() override;

        void bindGraphicsPipeline(const std::shared_ptr<RenderLib::GraphicsPipeline> &graphicsPipeline) override;

    private:
        void setHasChanged(uint32_t commandBufferIndex);

    public:
        void clearChangedStatus(uint32_t commandBufferIndex);
        bool hasCommandBufferChanged(uint32_t commandBufferIndex);
    };

}