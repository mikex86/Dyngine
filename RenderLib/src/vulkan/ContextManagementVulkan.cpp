#include <RenderLib/GLFWWindow_Internal.hpp>
#include <unordered_set>
#include <optional>
#include <algorithm>

namespace RenderLib {

    RenderSystem *CreateRenderSystem(const RenderSystemDescriptor &renderSystemDescriptor) {
        ENSURE_VULKAN_BACKEND(renderSystemDescriptor);
        auto config = reinterpret_cast<VulkanRenderSystemConfig *>(renderSystemDescriptor.config);
        auto application = config->application;
        VkApplicationInfo appInfo{
                .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pApplicationName = application.applicationName.c_str(),
                .applicationVersion = VK_MAKE_VERSION(application.applicationVersionMajor,
                                                      application.applicationVersionMinor,
                                                      application.applicationVersionPatch),
                .pEngineName = application.engineName.c_str(),
                .engineVersion = VK_MAKE_VERSION(application.engineVersionMajor,
                                                 application.engineVersionMinor,
                                                 application.engineVersionPatch),
                .apiVersion = VK_API_VERSION_1_0
        };

        auto instanceLayerStrings = std::vector(config->instanceLayers);
        const char **instanceLayerCstrs;
        size_t numLayers;
        // Get instance layers
        {
            numLayers = instanceLayerStrings.size();
            instanceLayerCstrs = new const char *[numLayers];
            for (size_t i = 0; i < instanceLayerStrings.size(); i++) {
                instanceLayerCstrs[i] = instanceLayerStrings[i].c_str();
            }
        }

        auto instanceExtensionStrings = std::vector(config->instanceExtensions);
        const char **instanceExtensionCstrs;
        size_t numExtensions;
        // Get instance extensions
        {
            auto requiredInstanceExtensions = GetRequiredInstanceExtensions();
            instanceExtensionStrings.insert(instanceExtensionStrings.end(), requiredInstanceExtensions.begin(),
                                            requiredInstanceExtensions.end());

            numExtensions = instanceExtensionStrings.size();
            instanceExtensionCstrs = new const char *[numExtensions];
            for (size_t i = 0; i < instanceExtensionStrings.size(); i++) {
                instanceExtensionCstrs[i] = instanceExtensionStrings[i].c_str();
            }
        }

        auto requiredDeviceExtensions = GetRequiredDeviceExtensions();

        VkInstanceCreateInfo instanceCreateInfo{
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pApplicationInfo = &appInfo,
                // Layers
                .enabledLayerCount = static_cast<uint32_t>(numLayers),
                .ppEnabledLayerNames = instanceLayerCstrs,

                // Extensions
                .enabledExtensionCount = static_cast<uint32_t>(numExtensions),
                .ppEnabledExtensionNames = instanceExtensionCstrs,
        };
        auto *renderSystem = new VulkanRenderSystem;
        renderSystem->backend = renderSystemDescriptor.backend;

        renderSystem->instanceLayers = instanceLayerStrings;
        renderSystem->instanceExtensions = instanceExtensionStrings;
        renderSystem->deviceExtensions = requiredDeviceExtensions;
        renderSystem->deviceLayers = {};

        VULKAN_STATUS_VALIDATE_WITH_CALLBACK(vkCreateInstance(&instanceCreateInfo, nullptr, &renderSystem->vkInstance),
                                             "Failed to create Vulkan instance", {
                                                 delete[] instanceLayerCstrs;
                                                 delete[] instanceExtensionCstrs;
                                             });
        return renderSystem;
    }

    /**
     * @brief Returns a capability score for the physical vulkan device derived from the device limits
     * @param device
     * @return
     */
    static uint64_t GetComputeCapability(VkPhysicalDeviceProperties deviceProperties) {
        uint64_t score = 0;
        score += deviceProperties.limits.maxComputeSharedMemorySize;
        score += deviceProperties.limits.maxComputeWorkGroupCount[0] *
                 deviceProperties.limits.maxComputeWorkGroupCount[1] *
                 deviceProperties.limits.maxComputeWorkGroupCount[2];
        score += deviceProperties.limits.maxComputeWorkGroupInvocations;
        score += deviceProperties.limits.maxComputeWorkGroupSize[0] *
                 deviceProperties.limits.maxComputeWorkGroupSize[1] *
                 deviceProperties.limits.maxComputeWorkGroupSize[2];
        return score;
    }


    std::vector<RenderDevice *> GetRenderDevices(RenderSystem *renderSystem) {
        ENSURE_VULKAN_BACKEND_PTR(renderSystem);

        auto vkRenderSystem = reinterpret_cast<VulkanRenderSystem *>(renderSystem);
        uint32_t numPhysicalDevices{};
        vkEnumeratePhysicalDevices(vkRenderSystem->vkInstance,
                                   &numPhysicalDevices, nullptr);

        std::vector<VkPhysicalDevice> physicalDevices(numPhysicalDevices);
        vkEnumeratePhysicalDevices(vkRenderSystem->vkInstance,
                                   &numPhysicalDevices, physicalDevices.data());

        std::vector<RenderDevice *> renderDevices;
        for (auto &physicalDevice: physicalDevices) {
            VkPhysicalDeviceProperties deviceProperties{};
            vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
            uint64_t computeCapability = GetComputeCapability(deviceProperties);
            auto *device = new VulkanRenderDevice{
                    .vkPhysicalDevice = physicalDevice
            };
            {
                device->deviceName = std::string(deviceProperties.deviceName),
                device->isDiscreteGpu = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
                device->capability = computeCapability;
            }
            renderDevices.push_back(device);
        }
        return renderDevices;
    }

    RenderDevice *GetBestRenderDevice(RenderSystem *renderSystem) {
        ENSURE_VULKAN_BACKEND_PTR(renderSystem);
        auto vulkanRenderSystem = reinterpret_cast<VulkanRenderSystem *>(renderSystem);

        std::vector<RenderDevice *> devices = GetRenderDevices(renderSystem);
        if (devices.empty()) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "No graphics card was found");
        }
        VulkanRenderDevice *bestDevice = nullptr;
        auto deviceExtensions = vulkanRenderSystem->deviceExtensions;
        for (auto device: devices) {
            if (bestDevice == nullptr || device->capability > bestDevice->capability) {
                auto vulkanRenderDevice = reinterpret_cast<VulkanRenderDevice *>(device);
                if (!DeviceSupportsExtension(vulkanRenderDevice->vkPhysicalDevice, deviceExtensions)) {
                    continue;
                }
                bestDevice = vulkanRenderDevice;
            }
        }
        if (bestDevice == nullptr) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "No supported graphics card was found");
        }
        return bestDevice;
    }

    static uint32_t FindQueueFamily(VkPhysicalDevice device, VkQueueFlagBits queueFlagBits) {
        uint32_t queueFamilyCount{};
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());
        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            if (queueFamilyProperties[i].queueFlags & queueFlagBits) {
                return i;
            }
        }
        return UINT32_MAX;
    }

    static uint32_t FindPresentFamily(VkPhysicalDevice device, VkSurfaceKHR vkSurface) {
        VkBool32 presentSupport{};
        uint32_t queueFamilyCount{};
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());
        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vkSurface, &presentSupport);
            if (presentSupport) {
                return i;
            }
        }
        return UINT32_MAX;
    }

    static std::vector<VkDeviceQueueCreateInfo>
    MakeQueueCreateInfos(const std::unordered_set<uint32_t> &queueFamilyIndices) {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
        for (auto queueFamilyIndex: queueFamilyIndices) {
            float queuePriority = 1.0f;
            VkDeviceQueueCreateInfo queueCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .queueFamilyIndex = queueFamilyIndex,
                    .queueCount = 1,
                    .pQueuePriorities = &queuePriority,
            };
            queueCreateInfos.push_back(queueCreateInfo);
        }
        return queueCreateInfos;
    }

    static VkDevice CreateDevice(const VulkanRenderSystem *renderSystem,
                                 const VulkanRenderDevice *renderDevice,
                                 const std::unordered_set<uint32_t> &queueFamilyIndices) {
        auto vkPhysicalDevice = renderDevice->vkPhysicalDevice;

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        {

            queueCreateInfos = MakeQueueCreateInfos(queueFamilyIndices);
        }

        std::vector<std::string> deviceExtensions = renderSystem->deviceExtensions;
        std::vector<std::string> deviceLayers = renderSystem->deviceLayers;

        auto numDeviceExtensions = static_cast<uint32_t>(deviceExtensions.size());
        auto numDeviceLayers = static_cast<uint32_t>(deviceLayers.size());
        char **deviceExtensionsCstrings = new char *[numDeviceExtensions];
        char **deviceLayersCstrings = new char *[numDeviceLayers];
        for (uint32_t i = 0; i < numDeviceExtensions; i++) {
            deviceExtensionsCstrings[i] = const_cast<char *>(deviceExtensions[i].c_str());
        }
        for (uint32_t i = 0; i < numDeviceLayers; i++) {
            deviceLayersCstrings[i] = const_cast<char *>(deviceLayers[i].c_str());
        }
        VkDevice vkDevice;
        {
            VkPhysicalDeviceFeatures deviceFeatures{};
            VkDeviceCreateInfo deviceCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
                    .pQueueCreateInfos = queueCreateInfos.data(),
                    .enabledLayerCount = numDeviceLayers,
                    .ppEnabledLayerNames = deviceLayersCstrings,
                    .enabledExtensionCount = numDeviceExtensions,
                    .ppEnabledExtensionNames = deviceExtensionsCstrings,
                    .pEnabledFeatures = &deviceFeatures,
            };

            VULKAN_STATUS_VALIDATE_WITH_CALLBACK(
                    vkCreateDevice(vkPhysicalDevice, &deviceCreateInfo, nullptr, &vkDevice),
                    "Create vulkan device failed",
                    {
                        delete[] deviceExtensionsCstrings;
                        delete[] deviceLayersCstrings;
                    }
            );
        }
        delete[] deviceExtensionsCstrings;
        delete[] deviceLayersCstrings;
        return vkDevice;
    }

    static VkQueue RetrieveQueue(VkDevice device, uint32_t queueFamilyIndex) {
        VkQueue vkQueue;
        {
            vkGetDeviceQueue(device, queueFamilyIndex, 0, &vkQueue);
        }
        return vkQueue;
    }

    static VulkanSwapChainSupportDetails
    GetSwapChainSupportDetails(VulkanRenderDevice *vulkanRenderDevice, VkSurfaceKHR surface) {
        VulkanSwapChainSupportDetails swapChainSupportDetails{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkanRenderDevice->vkPhysicalDevice, surface,
                                                  &swapChainSupportDetails.surfaceCapabilities);
        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanRenderDevice->vkPhysicalDevice, surface, &formatCount, nullptr);
        if (formatCount != 0) {
            swapChainSupportDetails.surfaceFormats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanRenderDevice->vkPhysicalDevice, surface, &formatCount,
                                                 swapChainSupportDetails.surfaceFormats.data());
        }
        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanRenderDevice->vkPhysicalDevice, surface, &presentModeCount,
                                                  nullptr);
        if (presentModeCount != 0) {
            swapChainSupportDetails.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanRenderDevice->vkPhysicalDevice, surface, &presentModeCount,
                                                      swapChainSupportDetails.presentModes.data());
        }
        return swapChainSupportDetails;
    }


    static std::optional<VkSurfaceFormatKHR>
    ChooseSurfaceFormat(const VulkanSwapChainSupportDetails &swapChainSupportDetails) {
        for (const auto &surfaceFormat: swapChainSupportDetails.surfaceFormats) {
            if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
                surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return surfaceFormat;
            }
        }
        return {};
    }

    static std::optional<VkPresentModeKHR>
    ChoosePresentMode(const VulkanSwapChainSupportDetails &swapChainSupportDetails) {
        for (const auto &presentMode: swapChainSupportDetails.presentModes) {
            if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return presentMode;
            }
        }
        return {};
    }

    static VkExtent2D
    ChooseExtent(GLFWWindowImpl *window, const VulkanSwapChainSupportDetails &swapChainSupportDetails) {
        auto capabilities = swapChainSupportDetails.surfaceCapabilities;
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(window->glfwWindow, &width, &height);
            VkExtent2D actualExtent{
                    .width = static_cast<uint32_t>(width),
                    .height = static_cast<uint32_t>(height)
            };
            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                            capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                             capabilities.maxImageExtent.height);
            return actualExtent;
        }
    }

    static uint32_t GetImageCount(const VulkanSwapChainSupportDetails &details) {
        uint32_t imageCount = details.surfaceCapabilities.minImageCount + 1;
        if (details.surfaceCapabilities.maxImageCount > 0 && imageCount > details.surfaceCapabilities.maxImageCount) {
            imageCount = details.surfaceCapabilities.maxImageCount;
        }
        return imageCount;
    }

    static VulkanQueueFamilyIndices
    GetQueueFamilies(VulkanRenderSystem *vulkanRenderSystem, VulkanRenderDevice *vulkanRenderDevice,
                     VkSurfaceKHR vkSurface) {
        uint32_t graphicsQueueFamilyIndex = FindQueueFamily(vulkanRenderDevice->vkPhysicalDevice,
                                                            VK_QUEUE_GRAPHICS_BIT);
        uint32_t computeQueueFamilyIndex = FindQueueFamily(vulkanRenderDevice->vkPhysicalDevice, VK_QUEUE_COMPUTE_BIT);
        uint32_t presentQueueFamilyIndex = FindPresentFamily(vulkanRenderDevice->vkPhysicalDevice, vkSurface);

        if (graphicsQueueFamilyIndex == UINT32_MAX) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "Graphics queue family index not found");
        }
        if (computeQueueFamilyIndex == UINT32_MAX) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "Compute queue family index not found");
        }
        if (presentQueueFamilyIndex == UINT32_MAX) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "Present queue family index not found");
        }
        return VulkanQueueFamilyIndices{
                .graphicsQueueFamilyIndex = graphicsQueueFamilyIndex,
                .presentQueueFamilyIndex = presentQueueFamilyIndex,
                .computeQueueFamilyIndex = computeQueueFamilyIndex,
                .uniqueQueueFamilyIndices = {
                        graphicsQueueFamilyIndex,
                        presentQueueFamilyIndex,
                        computeQueueFamilyIndex
                },
        };
    }

    static std::vector<VkImage> GetSwapChainImages(VkDevice vkDevice, VkSwapchainKHR vkSwapChainKHR) {
        uint32_t imageCount = 0;
        vkGetSwapchainImagesKHR(vkDevice, vkSwapChainKHR, &imageCount, nullptr);
        std::vector<VkImage> swapChainImages(imageCount);
        vkGetSwapchainImagesKHR(vkDevice, vkSwapChainKHR, &imageCount, swapChainImages.data());
        return swapChainImages;
    }

    static std::vector<VkImageView>
    CreateSwapChainImageViews(VkFormat swapChainImageFormat, VkDevice vkDevice, const std::vector<VkImage> &images) {
        std::vector<VkImageView> swapChainImageViews(images.size());
        for (uint32_t i = 0; i < images.size(); i++) {
            VkImageViewCreateInfo createInfo{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .image = images[i],
                    .viewType = VK_IMAGE_VIEW_TYPE_2D,
                    .format = swapChainImageFormat,
                    .components = {
                            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                            .a = VK_COMPONENT_SWIZZLE_IDENTITY
                    },
                    .subresourceRange = {
                            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                            .baseMipLevel = 0,
                            .levelCount = 1,
                            .baseArrayLayer = 0,
                            .layerCount = 1
                    }
            };
            VULKAN_STATUS_VALIDATE(
                    vkCreateImageView(vkDevice, &createInfo, nullptr, &swapChainImageViews[i]),
                    "Failed to create image view"
            );
        }
        return swapChainImageViews;
    }

    static VulkanSwapChain *CreateSwapChain(GLFWWindowImpl *window,
                                            VkDevice vkDevice,
                                            VulkanRenderDevice *vulkanRenderDevice,
                                            VulkanQueueFamilyIndices queueFamilyIndices,
                                            VkSurfaceKHR vkSurface) {
        auto swapChainSupportDetails = GetSwapChainSupportDetails(vulkanRenderDevice, vkSurface);
        auto surfaceFormat = ChooseSurfaceFormat(swapChainSupportDetails);
        if (!surfaceFormat) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "No suitable surface format found");
        }
        auto presentMode = ChoosePresentMode(swapChainSupportDetails);
        if (!presentMode) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "No suitable present mode found");
        }
        auto extent = ChooseExtent(window, swapChainSupportDetails);
        uint32_t imageCount = GetImageCount(swapChainSupportDetails);

        VkSwapchainCreateInfoKHR createInfo{
                .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                .surface = vkSurface,
                .minImageCount = imageCount,
                .imageFormat = surfaceFormat->format,
                .imageColorSpace = surfaceFormat->colorSpace,
                .imageExtent = extent,
                .imageArrayLayers = 1,
                .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        };
        std::vector<uint32_t> uniqueQueueFamilyIndices;
        uniqueQueueFamilyIndices.insert(
                uniqueQueueFamilyIndices.end(),
                queueFamilyIndices.uniqueQueueFamilyIndices.begin(),
                queueFamilyIndices.uniqueQueueFamilyIndices.end()
        );
        if (uniqueQueueFamilyIndices.empty()) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "No queue family indices found");
        }
        if (uniqueQueueFamilyIndices.size() == 1) {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = static_cast<uint32_t>(uniqueQueueFamilyIndices.size());
            createInfo.pQueueFamilyIndices = uniqueQueueFamilyIndices.data();
        }
        createInfo.preTransform = swapChainSupportDetails.surfaceCapabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = *presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        VkSwapchainKHR vkSwapChainKhr;
        if (vkCreateSwapchainKHR(vkDevice, &createInfo, nullptr, &vkSwapChainKhr) != VK_SUCCESS) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "Failed to create swap chain");
        }

        auto swapChainImages = GetSwapChainImages(vkDevice, vkSwapChainKhr);
        auto swapChainImageViews = CreateSwapChainImageViews(surfaceFormat->format, vkDevice, swapChainImages);

        auto swapChain = new VulkanSwapChain;
        {
            swapChain->vkDevice = vkDevice;
            swapChain->vkSwapChain = vkSwapChainKhr;
            swapChain->swapChainImages = swapChainImages;
            swapChain->swapChainImageViews = swapChainImageViews;
            swapChain->swapChainExtent = extent;
            swapChain->swapChainImageFormat = surfaceFormat->format;
        }
        return swapChain;
    }

    RenderContext *CreateRenderContext(Window *window, RenderSystem *renderSystem, RenderDevice *renderDevice) {
        ENSURE_VULKAN_BACKEND_PTR(renderSystem);
        ENSURE_VULKAN_BACKEND_PTR(window);

        auto vulkanRenderSystem = reinterpret_cast<VulkanRenderSystem *>(renderSystem);
        auto vulkanRenderDevice = reinterpret_cast<VulkanRenderDevice *>(renderDevice);

        auto glfwWindow = reinterpret_cast<GLFWWindowImpl *>(window);

        VkSurfaceKHR vkSurface;
        VULKAN_STATUS_VALIDATE(
                glfwCreateWindowSurface(vulkanRenderSystem->vkInstance, glfwWindow->glfwWindow, nullptr, &vkSurface),
                "Failed to create window surface"
        );

        VulkanQueueFamilyIndices queueFamilyIndices = GetQueueFamilies(vulkanRenderSystem, vulkanRenderDevice,
                                                                       vkSurface);

        auto graphicsQueueFamilyIndex = queueFamilyIndices.graphicsQueueFamilyIndex,
                computeQueueFamilyIndex = queueFamilyIndices.computeQueueFamilyIndex,
                presentQueueFamilyIndex = queueFamilyIndices.presentQueueFamilyIndex;

        VkDevice vkDevice = CreateDevice(vulkanRenderSystem, vulkanRenderDevice,
                                         {graphicsQueueFamilyIndex, computeQueueFamilyIndex, presentQueueFamilyIndex});

        VkQueue graphicsQueue = RetrieveQueue(vkDevice, graphicsQueueFamilyIndex);
        VkQueue computeQueue = RetrieveQueue(vkDevice, computeQueueFamilyIndex);
        VkQueue presentQueue = RetrieveQueue(vkDevice, presentQueueFamilyIndex);

        VulkanSwapChain *swapChain = CreateSwapChain(
                glfwWindow,
                vkDevice,
                vulkanRenderDevice,
                queueFamilyIndices,
                vkSurface
        );

        auto vulkanRenderContext = new VulkanRenderContext;
        {
            vulkanRenderContext->backend = VULKAN;
            vulkanRenderContext->vkInstance = vulkanRenderSystem->vkInstance;
            vulkanRenderContext->vkDevice = vkDevice;
            vulkanRenderContext->vkSurface = vkSurface;
            vulkanRenderContext->graphicsQueue = graphicsQueue;
            vulkanRenderContext->presentQueue = presentQueue;
            vulkanRenderContext->computeQueue = computeQueue;
            vulkanRenderContext->swapChain = swapChain;
        }
        return vulkanRenderContext;
    }

    std::vector<std::string> GetRequiredDeviceExtensions() {
        return {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
    }

    std::string GetRenderSystemBackendName(RenderSystemBackend backend) {
        switch (backend) {
            case RenderSystemBackend::VULKAN:
                return "Vulkan";
            default:
                return "Unknown";
        }
    }

    Window::Window(RenderSystemBackend backend) : backend(backend) {}

    VulkanRenderContext::~VulkanRenderContext() {
        delete swapChain;
        vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
        vkDestroyDevice(vkDevice, nullptr);
        vkDestroyInstance(vkInstance, nullptr);
    }

    VulkanSwapChain::~VulkanSwapChain() {
        for (auto &swapChainImageView : swapChainImageViews) {
            vkDestroyImageView(vkDevice, swapChainImageView, nullptr);
        }
        vkDestroySwapchainKHR(vkDevice, vkSwapChain, nullptr);
    }
}

