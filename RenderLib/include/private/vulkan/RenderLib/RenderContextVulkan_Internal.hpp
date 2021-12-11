#pragma once

#include <RenderLib/RenderLibVulkan_Internal.hpp>
#include <ErrorHandling/IllegalStateException.hpp>
#include <RenderLib/RenderContextVulkan.hpp>
#include <vulkan/vulkan.h>
#include <string>
#include <unordered_set>

namespace RenderLib {

    /**
     * @param result the vulkan status result
     * @return the human readable name of the vulkan result status code
     */
    std::string GetVulkanResultString(VkResult result);


    struct VulkanRenderSystem : public RenderSystem {
        VkInstance vkInstance;
        std::vector<std::string> instanceExtensions;
        std::vector<std::string> instanceLayers;
        std::vector<std::string> deviceExtensions;
        std::vector<std::string> deviceLayers;
    };

    struct VulkanRenderDevice : public RenderDevice {
        VkPhysicalDevice vkPhysicalDevice{};
    };

    struct VulkanSwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        std::vector<VkSurfaceFormatKHR> surfaceFormats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct GLFWWindowImpl;

    struct VulkanSwapChain {
        VkDevice vkDevice;
        VkPhysicalDevice vkPhysicalDevice;
        VkSwapchainKHR vkSwapChain;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;
        VkExtent2D swapChainExtent;
        VkFormat swapChainImageFormat;
        std::shared_ptr<GLFWWindowImpl> window;
        bool disposed = false;

        inline void dispose();

        virtual ~VulkanSwapChain();
    };

    struct VulkanQueueFamilyIndices {
        uint32_t graphicsQueueFamilyIndex;
        uint32_t presentQueueFamilyIndex;
        uint32_t computeQueueFamilyIndex;
        std::unordered_set<uint32_t> uniqueQueueFamilyIndices;
    };

    struct VulkanCommandBuffer;

    struct VulkanGraphicsPipeline;

    struct VulkanFrameBuffer : public FrameBuffer {
    private:
        bool disposed = false;
    public:
        VkDevice vkDevice;
        std::vector<VkFramebuffer> frameBuffers;

        VulkanFrameBuffer(VkDevice vkDevice,
                          std::vector<VkFramebuffer> frameBuffers);

        void dispose();

        ~VulkanFrameBuffer() override;
    };


    struct VulkanRenderContext : public RenderContext {

    public:
        VkInstance vkInstance;
        VkDevice vkDevice;
        VkPhysicalDevice vkPhysicalDevice;
        VkSurfaceKHR vkSurface;
        VkRenderPass vkRenderPass;
        VkCommandPool vkCommandPool;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        VkQueue computeQueue;
        VkSemaphore imageAvailableSemaphore;
        VkSemaphore renderFinishedSemaphore;
        std::shared_ptr<VulkanSwapChain> vulkanSwapChain;
        VulkanQueueFamilyIndices queueFamilyIndices;
        std::shared_ptr<VulkanFrameBuffer> vulkanFrameBuffer;
        std::vector<std::shared_ptr<VulkanGraphicsPipeline>> vulkanGraphicsPipelines;
        std::vector<std::shared_ptr<VulkanCommandBuffer>> vulkanCommandBuffers;

        uint32_t currentImageIndex;

        inline void beginFrame() override;

        inline void drawFrame(const std::shared_ptr<RenderLib::CommandBuffer> &commandBuffer) override;

        inline void endFrame() override;

        inline void synchronize() override;

        ~VulkanRenderContext() override;

    private:
        inline void submitCommandBuffer(const std::shared_ptr<VulkanCommandBuffer> &commandBuffer);

        [[nodiscard]] inline uint32_t acquireNextImage();

    };

    /**
     * @return the instance extensions required for the vulkan RenderLib Implementation
     */
    std::vector<std::string> GetRequiredInstanceExtensions();

    /**
     * @return the device extensions required for the vulkan RenderLib Implementation
     */
    std::vector<std::string> GetRequiredDeviceExtensions();

    /**
     * @return true if the specified device supports all of the specified extensions, else false
     */
    bool DeviceSupportsExtension(VkPhysicalDevice device, const std::vector<std::string> &extensions);

}