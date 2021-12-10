#include <RenderLib/GLFWWindow_Internal.hpp>
#include <RenderLib/RenderContextVulkan_Internal.hpp>
#include <memory>

namespace RenderLib {

    static bool glfwInitialized = false;

    inline static void EnsureGlfwInitialized() {
        if (!glfwInitialized) {
            glfwInit();
            glfwInitialized = true;
        }
    }

    std::vector<std::string> GetRequiredInstanceExtensions() {
        std::vector<std::string> requiredExtensions{};
        EnsureGlfwInitialized();

        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensionNames = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        for (uint32_t i = 0; i < glfwExtensionCount; i++) {
            requiredExtensions.emplace_back(glfwExtensionNames[i]);
        }
        return requiredExtensions;
    }

    bool DeviceSupportsExtension(VkPhysicalDevice device, const std::vector<std::string> &extensions) {
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        for (const auto &extension: extensions) {
            bool found = false;
            for (const auto &availableExtension: availableExtensions) {
                if (strcmp(extension.c_str(), availableExtension.extensionName) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return false;
            }
        }
        return true;
    }

    std::shared_ptr<Window> CreateNewWindow(const std::string &title, int width, int height) {
        EnsureGlfwInitialized();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        GLFWwindow *glfwWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        return std::make_shared<GLFWWindowImpl>(glfwWindow);
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

    EXCEPTION_TYPE_DEFAULT_IMPL(VulkanStatusException);
}