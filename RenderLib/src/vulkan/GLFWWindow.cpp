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

    EXCEPTION_TYPE_DEFAULT_IMPL(VulkanStatusException);


    int GLFWWindowImpl::getWidth() {
        int width;
        glfwGetWindowSize(glfwWindow, &width, nullptr);
        return width;
    }

    int GLFWWindowImpl::getHeight() {
        int height;
        glfwGetWindowSize(glfwWindow, nullptr, &height);
        return height;
    }

    void GLFWWindowImpl::show() {
        glfwShowWindow(glfwWindow);
    }

    void GLFWWindowImpl::update() {
        glfwPollEvents();
    }

    void GLFWWindowImpl::close() {
        glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
    }

    bool GLFWWindowImpl::shouldClose() {
        return glfwWindowShouldClose(glfwWindow);
    }

    GLFWWindowImpl::~GLFWWindowImpl() {
        glfwDestroyWindow(glfwWindow);
    }
}