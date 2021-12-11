#pragma once

#include <RenderLib/RenderLibVulkan_Internal.hpp>
#include <RenderLib/Windowing.hpp>
#include <GLFW/glfw3.h>

namespace RenderLib {

    class GLFWWindowImpl : public Window {
    public:
        GLFWwindow *glfwWindow;

        explicit GLFWWindowImpl(GLFWwindow *windowHandle) : glfwWindow(windowHandle),
                                                            Window(RenderSystemBackend::VULKAN) {
        }

    public:

        int getWidth() override;

        int getHeight() override;

        void show() override;

        void update() override;

        void close() override;

        bool shouldClose() override;

        ~GLFWWindowImpl();
    };

}
