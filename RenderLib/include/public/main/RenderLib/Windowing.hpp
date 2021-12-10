#pragma once

#include <RenderLib/RenderContext.hpp>
#include <memory>

namespace RenderLib {

    class Window {

    public:
        RenderSystemBackend backend;

        explicit Window(RenderSystemBackend backend);

        virtual void show() = 0;

        virtual void update() = 0;

        virtual void close() = 0;

        virtual bool shouldClose() = 0;
    };

    /**
     *
     * @brief Create a window
     * @param renderContext the context to use for rendering window contents
     * @param title the title of the window
     * @param width the width of the window
     * @param height the height of the window
     * @return the created Window
     */
    std::shared_ptr<Window> CreateNewWindow(const std::string &title, int width, int height);

}