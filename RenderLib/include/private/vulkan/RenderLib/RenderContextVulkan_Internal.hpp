#pragma once

#include <ErrorHandling/IllegalStateException.hpp>
#include <RenderLib/RenderContextVulkan.hpp>
#include <vulkan/vulkan.h>
#include <string>
#include <unordered_set>

#define RENDERLIB_DEBUG_VALIDATIONS

#ifdef RENDERLIB_DEBUG_VALIDATIONS
#define ENSURE_VULKAN_BACKEND_PTR(handle) \
    if ((handle)->backend != RenderSystemBackend::VULKAN) \
    RAISE_EXCEPTION(errorhandling::IllegalStateException, \
                    std::string("Handle "  #handle  " has an unexpected backend: ") + \
                    GetRenderSystemBackendName((handle)->backend) + ", should be Vulkan instead!")

#define ENSURE_VULKAN_BACKEND(handle) \
    if ((handle).backend != RenderSystemBackend::VULKAN) \
    RAISE_EXCEPTION(errorhandling::IllegalStateException, \
                    std::string("Handle " #handle " has an unexpected backend: ") + \
                    GetRenderSystemBackendName((handle).backend) + ", should be Vulkan instead!")
#else
#define ENSURE_VULKAN_BACKEND_PTR(renderSystem)
#define ENSURE_VULKAN_BACKEND(renderSystem)
#endif

#ifdef RENDERLIB_DEBUG_VALIDATIONS
#define VULKAN_STATUS_VALIDATE_WITH_CALLBACK(errorStatus, errorMessage, errorCallback) \
{                                                                                      \
auto status = (errorStatus);                                                                                       \
if ((status) != VK_SUCCESS) {                                       \
    errorCallback;                                                                     \
    RAISE_EXCEPTION(VulkanStatusException, std::string(errorMessage) + ": " + RenderLib::GetVulkanResultString(status)); \
} \
}
#else
#define VULKAN_STATUS_VALIDATE_WITH_CALLBACK(errorStatus, errorMessage, errorCallback) if ((errorStatus) != VK_SUCCESS) { errorCallBack;}
#endif

#define VULKAN_STATUS_VALIDATE(errorStatus, errorMessage) VULKAN_STATUS_VALIDATE_WITH_CALLBACK(errorStatus, errorMessage, {})

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

    struct VulkanSwapChain {
        VkDevice vkDevice;
        VkSwapchainKHR vkSwapChain;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;
        VkExtent2D swapChainExtent;
        VkFormat swapChainImageFormat;

        virtual ~VulkanSwapChain();
    };

    struct VulkanCommandBuffer;

    struct VulkanRenderContext : public RenderContext {

    public:
        VkInstance vkInstance;
        VkDevice vkDevice;
        VkSurfaceKHR vkSurface;
        VkRenderPass vkRenderPass;
        VkCommandPool vkCommandPool;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        VkQueue computeQueue;
        VkSemaphore imageAvailableSemaphore;
        VkSemaphore renderFinishedSemaphore;
        std::shared_ptr<VulkanSwapChain> vulkanSwapChain;

        uint32_t currentImageIndex;

        void beginFrame() override;

        void drawFrame(const std::shared_ptr<RenderLib::CommandBuffer> &commandBuffer) override;

        void endFrame() override;

        void synchronize() override;

        ~VulkanRenderContext() override;

    private:
        void submitCommandBuffer(const std::shared_ptr<VulkanCommandBuffer> &commandBuffer);

        [[nodiscard]] uint32_t acquireNextImage() const;
    };

    struct VulkanQueueFamilyIndices {
        uint32_t graphicsQueueFamilyIndex;
        uint32_t presentQueueFamilyIndex;
        uint32_t computeQueueFamilyIndex;
        std::unordered_set<uint32_t> uniqueQueueFamilyIndices;
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