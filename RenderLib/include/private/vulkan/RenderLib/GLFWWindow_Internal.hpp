#pragma once

#include <RenderLib/ContextManagementVulkan_Internal.hpp>
#include <GLFW/glfw3.h>

namespace RenderLib {

    class GLFWWindowImpl : public Window {
    public:
        GLFWwindow *glfwWindow;

        explicit GLFWWindowImpl(GLFWwindow *windowHandle) : glfwWindow(windowHandle), Window(RenderSystemBackend::VULKAN) {
        }

    public:

        void show() override {
            glfwShowWindow(glfwWindow);
        }

        void update() override {
            glfwPollEvents();
        }

        void close() override {
            glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
        }

        bool shouldClose() override {
            return glfwWindowShouldClose(glfwWindow);
        }

        ~GLFWWindowImpl() {
            glfwDestroyWindow(glfwWindow);
        }
    };

}
