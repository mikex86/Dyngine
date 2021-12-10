#pragma once

#include <ErrorHandling/IllegalStateException.hpp>
#include <RenderLib/ContextManagementVulkan.hpp>
#include <vulkan/vulkan.h>
#include <string>
#include <unordered_set>

#define RENDERLIB_DEBUG_VALIDATIONS

#ifdef RENDERLIB_DEBUG_VALIDATIONS
#define ENSURE_VULKAN_BACKEND_PTR(handle) \
    if ((handle)->backend != RenderSystemBackend::VULKAN) \
    RAISE_EXCEPTION(errorhandling::IllegalStateException, \
                    std::string("Handle"  #handle  " has an unexpected backend: ") + \
                    GetRenderSystemBackendName((handle)->backend) + ", should be Vulkan instead!")

#define ENSURE_VULKAN_BACKEND(handle) \
    if ((handle).backend != RenderSystemBackend::VULKAN) \
    RAISE_EXCEPTION(errorhandling::IllegalStateException, \
                    std::string("Handle" #handle " has an unexpected backend: ") + \
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

    struct VulkanRenderContext : public RenderContext {

        VkInstance vkInstance;
        VkDevice vkDevice;
        VkSurfaceKHR vkSurface;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        VkQueue computeQueue;

        std::shared_ptr<VulkanSwapChain> swapChain;

        ~VulkanRenderContext() override;
        
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