#include <RenderLib/RenderContextVulkan_Internal.hpp>
#include <RenderLib/GraphicsPipelineVulkan_Internal.hpp>
#include <RenderLib/GLFWWindow_Internal.hpp>
#include <RenderLib/CommandBufferVulkan_Internal.hpp>
#include <unordered_set>
#include <optional>
#include <algorithm>
#include <memory>
#include <utility>

namespace RenderLib {

    std::shared_ptr<RenderSystem> CreateRenderSystem(const RenderSystemDescriptor &renderSystemDescriptor) {
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
        VkInstance vkInstance;

        VULKAN_STATUS_VALIDATE_WITH_CALLBACK(vkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance),
                                             "Failed to create Vulkan instance", {
                                                 delete[] instanceLayerCstrs;
                                                 delete[] instanceExtensionCstrs;
                                             });

        auto vulkanRenderSystem = new VulkanRenderSystem{};
        {
            vulkanRenderSystem->backend = renderSystemDescriptor.backend;
            vulkanRenderSystem->vkInstance = vkInstance;
            vulkanRenderSystem->instanceLayers = instanceLayerStrings;
            vulkanRenderSystem->instanceExtensions = instanceExtensionStrings;
            vulkanRenderSystem->deviceExtensions = requiredDeviceExtensions;
            vulkanRenderSystem->deviceLayers = {};
        }
        return std::shared_ptr<VulkanRenderSystem>(vulkanRenderSystem);
    }

    /**
     * @brief Returns a capability score for the physical vulkan device derived from the device limits
     * @param device
     * @return
     */
    inline static uint64_t GetComputeCapability(VkPhysicalDeviceProperties deviceProperties) {
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


    inline std::vector<std::shared_ptr<RenderDevice>>
    GetRenderDevices(const std::shared_ptr<RenderSystem> &renderSystem) {
        ENSURE_VULKAN_BACKEND_PTR(renderSystem);

        auto vkRenderSystem = std::dynamic_pointer_cast<VulkanRenderSystem>(renderSystem);
        uint32_t numPhysicalDevices{};
        vkEnumeratePhysicalDevices(vkRenderSystem->vkInstance,
                                   &numPhysicalDevices, nullptr);

        std::vector<VkPhysicalDevice> physicalDevices(numPhysicalDevices);
        vkEnumeratePhysicalDevices(vkRenderSystem->vkInstance,
                                   &numPhysicalDevices, physicalDevices.data());

        std::vector<std::shared_ptr<RenderDevice>> renderDevices;
        for (auto &physicalDevice: physicalDevices) {
            VkPhysicalDeviceProperties deviceProperties{};
            vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
            uint64_t computeCapability = GetComputeCapability(deviceProperties);
            auto *device = new VulkanRenderDevice{};
            {
                device->vkPhysicalDevice = physicalDevice;
                device->deviceName = std::string(deviceProperties.deviceName),
                device->isDiscreteGpu = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
                device->capability = computeCapability;
            }
            renderDevices.emplace_back(device);
        }
        return renderDevices;
    }

    std::shared_ptr<RenderDevice> GetBestRenderDevice(const std::shared_ptr<RenderSystem> &renderSystem) {
        ENSURE_VULKAN_BACKEND_PTR(renderSystem);
        auto vulkanRenderSystem = std::dynamic_pointer_cast<VulkanRenderSystem>(renderSystem);

        std::vector<std::shared_ptr<RenderDevice>> devices = GetRenderDevices(renderSystem);
        if (devices.empty()) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "No graphics card was found");
        }
        std::shared_ptr<VulkanRenderDevice> bestDevice = nullptr;
        auto deviceExtensions = vulkanRenderSystem->deviceExtensions;
        for (const auto &device: devices) {
            if (bestDevice == nullptr || device->capability > bestDevice->capability) {
                auto vulkanRenderDevice = std::dynamic_pointer_cast<VulkanRenderDevice>(device);
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

    inline static uint32_t FindQueueFamily(VkPhysicalDevice device, VkQueueFlagBits queueFlagBits) {
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

    inline static uint32_t FindPresentFamily(VkPhysicalDevice device, VkSurfaceKHR vkSurface) {
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

    inline static std::vector<VkDeviceQueueCreateInfo>
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

    inline static VkDevice CreateDevice(const std::shared_ptr<VulkanRenderSystem> &renderSystem,
                                        const std::shared_ptr<VulkanRenderDevice> &renderDevice,
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

    inline static VkQueue RetrieveQueue(VkDevice device, uint32_t queueFamilyIndex) {
        VkQueue vkQueue;
        {
            vkGetDeviceQueue(device, queueFamilyIndex, 0, &vkQueue);
        }
        return vkQueue;
    }

    inline static VulkanSwapChainSupportDetails
    GetSwapChainSupportDetails(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR surface) {
        VulkanSwapChainSupportDetails swapChainSupportDetails{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, surface,
                                                  &swapChainSupportDetails.surfaceCapabilities);
        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, surface, &formatCount, nullptr);
        if (formatCount != 0) {
            swapChainSupportDetails.surfaceFormats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, surface, &formatCount,
                                                 swapChainSupportDetails.surfaceFormats.data());
        }
        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, surface, &presentModeCount,
                                                  nullptr);
        if (presentModeCount != 0) {
            swapChainSupportDetails.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, surface, &presentModeCount,
                                                      swapChainSupportDetails.presentModes.data());
        }
        return swapChainSupportDetails;
    }


    inline static std::optional<VkSurfaceFormatKHR>
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

    inline static VkExtent2D
    ChooseExtent(const std::shared_ptr<GLFWWindowImpl> &window,
                 const VulkanSwapChainSupportDetails &swapChainSupportDetails) {
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

    inline static uint32_t GetImageCount(const VulkanSwapChainSupportDetails &details) {
        uint32_t imageCount = details.surfaceCapabilities.minImageCount + 1;
        if (details.surfaceCapabilities.maxImageCount > 0 && imageCount > details.surfaceCapabilities.maxImageCount) {
            imageCount = details.surfaceCapabilities.maxImageCount;
        }
        return imageCount;
    }

    inline static VulkanQueueFamilyIndices
    GetQueueFamilies(const std::shared_ptr<VulkanRenderSystem> &vulkanRenderSystem,
                     const std::shared_ptr<VulkanRenderDevice> &vulkanRenderDevice,
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

    inline static std::vector<VkImage> GetSwapChainImages(VkDevice vkDevice, VkSwapchainKHR vkSwapChain) {
        uint32_t imageCount = 0;
        vkGetSwapchainImagesKHR(vkDevice, vkSwapChain, &imageCount, nullptr);
        std::vector<VkImage> swapChainImages(imageCount);
        vkGetSwapchainImagesKHR(vkDevice, vkSwapChain, &imageCount, swapChainImages.data());
        return swapChainImages;
    }

    inline static std::vector<VkImageView>
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


    inline static VkAttachmentReference MakeColorAttachmentRef() {
        return VkAttachmentReference{
                .attachment = 0,
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };
    }

    inline static VkAttachmentDescription MakeColorAttachment(VkFormat imageFormat) {
        return VkAttachmentDescription{
                .format = imageFormat,
                .samples = VK_SAMPLE_COUNT_1_BIT, // Multi-sampling is not used (for now)
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, // Clear framebuffer before rendering
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };
    }

    inline static VkRenderPass
    CreateRenderPass(VkDevice vkDevice, const VkAttachmentReference &colorAttachmentRef,
                     const VkAttachmentDescription &colorAttachment) {

        VkSubpassDescription subPasses{
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .colorAttachmentCount = 1,
                .pColorAttachments = &colorAttachmentRef
        };

        VkSubpassDependency dependency{
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .dstSubpass = 0,
                .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = 0,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        };

        VkRenderPassCreateInfo renderPassInfo{
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .attachmentCount = 1,
                .pAttachments = &colorAttachment,
                .subpassCount = 1,
                .pSubpasses = &subPasses,
                .dependencyCount = 1,
                .pDependencies = &dependency
        };
        VkRenderPass vkRenderPass;
        VULKAN_STATUS_VALIDATE(
                vkCreateRenderPass(vkDevice, &renderPassInfo, nullptr, &vkRenderPass),
                "Failed to create render pass"
        );
        return vkRenderPass;
    }

    inline static std::shared_ptr<VulkanSwapChain> CreateSwapChain(const std::shared_ptr<GLFWWindowImpl> &window,
                                                                   VkDevice vkDevice,
                                                                   VkPhysicalDevice vkPhysicalDevice,
                                                                   VulkanQueueFamilyIndices queueFamilyIndices,
                                                                   VkSurfaceKHR vkSurface) {
        auto swapChainSupportDetails = GetSwapChainSupportDetails(vkPhysicalDevice, vkSurface);
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

        VkSwapchainKHR vkSwapChain;

        VkSwapchainCreateInfoKHR swapChainCreateInfo{
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
            swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapChainCreateInfo.queueFamilyIndexCount = 0;
            swapChainCreateInfo.pQueueFamilyIndices = nullptr;
        } else {
            swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapChainCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(uniqueQueueFamilyIndices.size());
            swapChainCreateInfo.pQueueFamilyIndices = uniqueQueueFamilyIndices.data();
        }

        swapChainCreateInfo.preTransform = swapChainSupportDetails.surfaceCapabilities.currentTransform;
        swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapChainCreateInfo.presentMode = *presentMode;
        swapChainCreateInfo.clipped = VK_TRUE;
        swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

        VULKAN_STATUS_VALIDATE(
                vkCreateSwapchainKHR(vkDevice, &swapChainCreateInfo, nullptr, &vkSwapChain),
                "Failed to create swap chain"
        );

        auto swapChainImages = GetSwapChainImages(vkDevice, vkSwapChain);
        auto swapChainImageViews = CreateSwapChainImageViews(surfaceFormat->format, vkDevice, swapChainImages);

        auto swapChain = new VulkanSwapChain;
        {
            swapChain->vkDevice = vkDevice;
            swapChain->vkPhysicalDevice = vkPhysicalDevice;
            swapChain->vkSwapChain = vkSwapChain;
            swapChain->swapChainImages = swapChainImages;
            swapChain->swapChainImageViews = swapChainImageViews;
            swapChain->swapChainExtent = extent;
            swapChain->swapChainImageFormat = surfaceFormat->format;
            swapChain->window = window;
        }
        return std::shared_ptr<VulkanSwapChain>(swapChain);
    }

    inline VkCommandPool CreateCommandPool(VkDevice vkDevice, const VulkanQueueFamilyIndices &queueFamilyIndices) {
        VkCommandPoolCreateInfo createInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = queueFamilyIndices.graphicsQueueFamilyIndex,
        };

        VkCommandPool vkCommandPool;
        VULKAN_STATUS_VALIDATE(vkCreateCommandPool(vkDevice, &createInfo, nullptr, &vkCommandPool),
                               "Failed to create command pool");
        return vkCommandPool;
    }

    inline static VkSemaphore CreateSemaphore(VkDevice device) {
        VkSemaphoreCreateInfo createInfo{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };
        VkSemaphore vkSemaphore;
        VULKAN_STATUS_VALIDATE(
                vkCreateSemaphore(device, &createInfo, nullptr, &vkSemaphore),
                "Failed to create semaphore"
        );
        return vkSemaphore;
    }


    VulkanFrameBuffer::VulkanFrameBuffer(VkDevice vkDevice,
                                         std::vector<VkFramebuffer> frameBuffers) : FrameBuffer(VULKAN),
                                                                                    vkDevice(vkDevice),
                                                                                    frameBuffers(
                                                                                            std::move(frameBuffers)) {
    }

    void VulkanFrameBuffer::dispose() {
        if (disposed) {
            return;
        }
        for (auto frameBuffer: frameBuffers) {
            vkDestroyFramebuffer(vkDevice, frameBuffer, nullptr);
        }
        disposed = true;
    }

    VulkanFrameBuffer::~VulkanFrameBuffer() {
        dispose();
    }

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

    static std::shared_ptr<VulkanFrameBuffer>
    CreateFrameBuffer(VkDevice vkDevice, VkRenderPass vkRenderPass, const std::shared_ptr<VulkanSwapChain> &swapChain) {
        auto swapChainImageViews = swapChain->swapChainImageViews;
        auto swapChainExtent = swapChain->swapChainExtent;
        auto frameBuffers = CreateFrameBuffers(vkDevice,
                                               vkRenderPass,
                                               swapChainImageViews,
                                               swapChainExtent);
        return std::make_shared<VulkanFrameBuffer>(vkDevice, frameBuffers);
    }

    std::shared_ptr<RenderContext>
    CreateRenderContext(const std::shared_ptr<Window> &window, const std::shared_ptr<RenderSystem> &renderSystem,
                        const std::shared_ptr<RenderDevice> &renderDevice) {
        ENSURE_VULKAN_BACKEND_PTR(renderSystem);
        ENSURE_VULKAN_BACKEND_PTR(window);

        auto vulkanRenderSystem = std::dynamic_pointer_cast<VulkanRenderSystem>(renderSystem);
        auto vulkanRenderDevice = std::dynamic_pointer_cast<VulkanRenderDevice>(renderDevice);

        auto glfwWindow = std::dynamic_pointer_cast<GLFWWindowImpl>(window);

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

        VkPhysicalDevice vkPhysicalDevice = vulkanRenderDevice->vkPhysicalDevice;
        VkDevice vkDevice = CreateDevice(vulkanRenderSystem, vulkanRenderDevice,
                                         {graphicsQueueFamilyIndex, computeQueueFamilyIndex, presentQueueFamilyIndex});

        VkQueue graphicsQueue = RetrieveQueue(vkDevice, graphicsQueueFamilyIndex);
        VkQueue computeQueue = RetrieveQueue(vkDevice, computeQueueFamilyIndex);
        VkQueue presentQueue = RetrieveQueue(vkDevice, presentQueueFamilyIndex);

        std::shared_ptr<VulkanSwapChain> swapChain = CreateSwapChain(
                glfwWindow,
                vkDevice,
                vkPhysicalDevice,
                queueFamilyIndices,
                vkSurface
        );

        auto colorAttachment = MakeColorAttachment(swapChain->swapChainImageFormat);
        auto colorAttachmentRef = MakeColorAttachmentRef();
        auto renderPass = CreateRenderPass(vkDevice, colorAttachmentRef, colorAttachment);
        auto commandPool = CreateCommandPool(vkDevice, queueFamilyIndices);

        auto imageAvailableSemaphore = CreateSemaphore(vkDevice);
        auto renderFinishedSemaphore = CreateSemaphore(vkDevice);

        auto vulkanRenderContext = new VulkanRenderContext;
        {
            vulkanRenderContext->backend = VULKAN;
            vulkanRenderContext->vkInstance = vulkanRenderSystem->vkInstance;
            vulkanRenderContext->vkDevice = vkDevice;
            vulkanRenderContext->vkPhysicalDevice = vkPhysicalDevice;
            vulkanRenderContext->vkSurface = vkSurface;
            vulkanRenderContext->vkRenderPass = renderPass;
            vulkanRenderContext->vkCommandPool = commandPool;
            vulkanRenderContext->graphicsQueue = graphicsQueue;
            vulkanRenderContext->presentQueue = presentQueue;
            vulkanRenderContext->computeQueue = computeQueue;
            vulkanRenderContext->vulkanSwapChain = swapChain;
            vulkanRenderContext->queueFamilyIndices = queueFamilyIndices;
            vulkanRenderContext->vulkanFrameBuffer = CreateFrameBuffer(vkDevice, renderPass, swapChain);
            vulkanRenderContext->imageAvailableSemaphore = imageAvailableSemaphore;
            vulkanRenderContext->renderFinishedSemaphore = renderFinishedSemaphore;
        }
        return std::shared_ptr<VulkanRenderContext>(vulkanRenderContext);
    }

    inline std::vector<std::string> GetRequiredDeviceExtensions() {
        return {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
    }

    inline std::string GetRenderSystemBackendName(RenderSystemBackend backend) {
        switch (backend) {
            case RenderSystemBackend::VULKAN:
                return "Vulkan";
            default:
                return "Unknown";
        }
    }

    Window::Window(RenderSystemBackend backend) : backend(backend) {}

    VulkanRenderContext::~VulkanRenderContext() {
        vulkanSwapChain = nullptr;
        vkDestroySemaphore(vkDevice, imageAvailableSemaphore, nullptr);
        vkDestroySemaphore(vkDevice, renderFinishedSemaphore, nullptr);
        vkDestroyRenderPass(vkDevice, vkRenderPass, nullptr);
        vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
        vkDestroyCommandPool(vkDevice, vkCommandPool, nullptr);
        vkDestroyDevice(vkDevice, nullptr);
        vkDestroyInstance(vkInstance, nullptr);
    }

    static inline void RecreateSwapChain(const std::shared_ptr<VulkanRenderContext> &renderContext) {
        vkDeviceWaitIdle(renderContext->vkDevice); // wait for async operations to finish

        auto swapChain = renderContext->vulkanSwapChain;
        auto vkDevice = swapChain->vkDevice;
        auto vkPhysicalDevice = swapChain->vkPhysicalDevice;
        auto glfwWindow = swapChain->window;
        auto vkSurface = renderContext->vkSurface;
        auto queueFamilyIndices = renderContext->queueFamilyIndices;
        renderContext->vulkanSwapChain->dispose();
        renderContext->vulkanSwapChain = nullptr;
        auto newSwapChain = CreateSwapChain(
                glfwWindow,
                vkDevice,
                vkPhysicalDevice,
                queueFamilyIndices,
                vkSurface
        );
        renderContext->vulkanSwapChain = newSwapChain;
    }

    static inline void RecreateRenderPass(const std::shared_ptr<VulkanRenderContext> &renderContext) {
        auto colorAttachment = MakeColorAttachment(renderContext->vulkanSwapChain->swapChainImageFormat);
        auto colorAttachmentRef = MakeColorAttachmentRef();
        auto renderPass = CreateRenderPass(renderContext->vkDevice, colorAttachmentRef, colorAttachment);
        vkDestroyRenderPass(renderContext->vkDevice, renderContext->vkRenderPass, nullptr);
        renderContext->vkRenderPass = renderPass;
    }

    static inline void RecreateGraphicsPipeline(const std::shared_ptr<VulkanGraphicsPipeline> &graphicsPipeline) {
        auto renderContext = graphicsPipeline->vulkanRenderContext;
        auto vertexFormat = graphicsPipeline->vertexFormat;
        auto pipelineLayout = graphicsPipeline->pipelineLayout;
        auto shaderProgram = graphicsPipeline->shaderProgram;
        graphicsPipeline->dispose();
        auto newGraphicsPipeline = CreateVulkanGraphicsPipelinePtr(renderContext, vertexFormat, pipelineLayout, shaderProgram);
        graphicsPipeline->transferStateFrom(newGraphicsPipeline);
        delete newGraphicsPipeline;
    }

    static inline void RecreateGraphicsPipeline(const std::shared_ptr<VulkanRenderContext> &renderContext) {
        auto pipelines = renderContext->vulkanGraphicsPipelines;
        for (auto &pipeline: pipelines) {
            RecreateGraphicsPipeline(pipeline);
        }
    }

    static inline void RecreateCommandBuffer(const std::shared_ptr<VulkanCommandBuffer> &commandBuffer) {
        commandBuffer->dispose();
        auto newCommandBuffer = CreateVulkanCommandBufferPtr(commandBuffer->vulkanRenderContext);
        commandBuffer->transferStateFrom(newCommandBuffer);
    }

    static inline void RecreateCommandBuffers(const std::shared_ptr<VulkanRenderContext> &renderContext) {
        auto nCommandBuffers = renderContext->vulkanCommandBuffers.size();

        for (size_t i = 0; i < nCommandBuffers; i++) {
            auto commandBuffer = renderContext->vulkanCommandBuffers[i];
            RecreateCommandBuffer(commandBuffer);
        }
    }

    static inline void RecreateFrameBuffer(const std::shared_ptr<VulkanRenderContext> &renderContext) {
        renderContext->vulkanFrameBuffer->dispose();
        renderContext->vulkanFrameBuffer = nullptr;
        auto newFrameBuffer = CreateFrameBuffer(renderContext->vkDevice,
                                                renderContext->vkRenderPass,
                                                renderContext->vulkanSwapChain);
        renderContext->vulkanFrameBuffer = newFrameBuffer;
    }

    static inline void ReCreateContext(const std::shared_ptr<VulkanRenderContext> &renderContext) {
        RecreateSwapChain(renderContext);
        RecreateRenderPass(renderContext);
        RecreateGraphicsPipeline(renderContext);
        RecreateFrameBuffer(renderContext);
        RecreateCommandBuffers(renderContext);
    }

    static inline void
    RecreateContextIfNecessary(VkResult vkResult, const std::shared_ptr<VulkanRenderContext> &renderContext) {
        if (vkResult == VK_ERROR_OUT_OF_DATE_KHR) {
            ReCreateContext(renderContext);
        } else if (vkResult != VK_SUCCESS && vkResult != VK_SUBOPTIMAL_KHR) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "Failed to acquire swap chain image!");
        }
    }

    inline uint32_t VulkanRenderContext::acquireNextImage() {
        uint32_t imageIndex;
        {
            RecreateContextIfNecessary(
                    vkAcquireNextImageKHR(vkDevice, vulkanSwapChain->vkSwapChain,
                                          std::numeric_limits<uint64_t>::max(),
                                          imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex),
                    std::shared_ptr<VulkanRenderContext>(this, [](VulkanRenderContext *) {})
            );
        }
        return imageIndex;
    }

    void VulkanRenderContext::submitCommandBuffer(const std::shared_ptr<VulkanCommandBuffer> &commandBuffer) {
        VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
        VkSubmitInfo submitInfo{
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .waitSemaphoreCount =1,
                .pWaitSemaphores = waitSemaphores,
                .pWaitDstStageMask = waitStages,
                .commandBufferCount = 1,
                .pCommandBuffers = &commandBuffer->commandBuffers[currentImageIndex],
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = signalSemaphores
        };
        VULKAN_STATUS_VALIDATE(
                vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE),
                "Failed to submit command buffer"
        );
        VkSwapchainKHR swapChains[] = {vulkanSwapChain->vkSwapChain};
        VkPresentInfoKHR presentInfo{
                .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = signalSemaphores,
                .swapchainCount = 1,
                .pSwapchains = swapChains,
                .pImageIndices = &currentImageIndex,
                .pResults = nullptr
        };

        RecreateContextIfNecessary(
                vkQueuePresentKHR(presentQueue, &presentInfo),
                std::shared_ptr<VulkanRenderContext>(this, [](VulkanRenderContext *) {})
        );
    }

    void VulkanRenderContext::beginFrame() {
        currentImageIndex = acquireNextImage();

    }

    void VulkanRenderContext::drawFrame(const std::shared_ptr<CommandBuffer> &commandBuffer) {
        ENSURE_VULKAN_BACKEND_PTR(commandBuffer);
        auto vulkanCommandBuffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(commandBuffer);
        submitCommandBuffer(vulkanCommandBuffer);
    }

    void VulkanRenderContext::synchronize() {
        vkDeviceWaitIdle(vkDevice);
    }

    void VulkanRenderContext::endFrame() {
    }

    void VulkanSwapChain::dispose() {
        if (disposed) {
            return;
        }
        for (auto &swapChainImageView: swapChainImageViews) {
            vkDestroyImageView(vkDevice, swapChainImageView, nullptr);
        }
        vkDestroySwapchainKHR(vkDevice, vkSwapChain, nullptr);
        disposed = true;
    }

    VulkanSwapChain::~VulkanSwapChain() {
        dispose();
    }

    std::string GetVulkanResultString(VkResult result) {
        switch (result) {
            case 0:
                return "VK_SUCCESS";
            case 1:
                return "VK_NOT_READY";
            case 2:
                return "VK_TIMEOUT";
            case 3:
                return "VK_EVENT_SET";
            case 4:
                return "VK_EVENT_RESET";
            case 5:
                return "VK_INCOMPLETE";
            case -1:
                return "VK_ERROR_OUT_OF_HOST_MEMORY";
            case -2:
                return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
            case -3:
                return "VK_ERROR_INITIALIZATION_FAILED";
            case -4:
                return "VK_ERROR_DEVICE_LOST";
            case -5:
                return "VK_ERROR_MEMORY_MAP_FAILED";
            case -6:
                return "VK_ERROR_LAYER_NOT_PRESENT";
            case -7:
                return "VK_ERROR_EXTENSION_NOT_PRESENT";
            case -8:
                return "VK_ERROR_FEATURE_NOT_PRESENT";
            case -9:
                return "VK_ERROR_INCOMPATIBLE_DRIVER";
            case -10:
                return "VK_ERROR_TOO_MANY_OBJECTS";
            case -11:
                return "VK_ERROR_FORMAT_NOT_SUPPORTED";
            case -12:
                return "VK_ERROR_FRAGMENTED_POOL";
            case -13:
                return "VK_ERROR_UNKNOWN";
            case -1000069000:
                return "VK_ERROR_OUT_OF_POOL_MEMORY";
            case -1000072003:
                return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
            case -1000161000:
                return "VK_ERROR_FRAGMENTATION";
            case -1000257000:
                return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
            case -1000000000:
                return "VK_ERROR_SURFACE_LOST_KHR";
            case -1000000001:
                return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
            case 1000001003:
                return "VK_SUBOPTIMAL_KHR";
            case -1000001004:
                return "VK_ERROR_OUT_OF_DATE_KHR";
            case -1000003001:
                return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
            case -1000011001:
                return "VK_ERROR_VALIDATION_FAILED_EXT";
            case -1000012000:
                return "VK_ERROR_INVALID_SHADER_NV";
            case -1000158000:
                return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
            case -1000174001:
                return "VK_ERROR_NOT_PERMITTED_EXT";
            case -1000255000:
                return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
            case 1000268000:
                return "VK_THREAD_IDLE_KHR";
            case 1000268001:
                return "VK_THREAD_DONE_KHR";
            case 1000268002:
                return "VK_OPERATION_DEFERRED_KHR";
            case 1000268003:
                return "VK_OPERATION_NOT_DEFERRED_KHR";
            case 1000297000:
                return "VK_PIPELINE_COMPILE_REQUIRED_EXT";
            case 0x7FFFFFFF:
                return "VK_RESULT_MAX_ENUM";
        }
    }
}

