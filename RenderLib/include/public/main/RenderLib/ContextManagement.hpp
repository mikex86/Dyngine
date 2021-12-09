#pragma once

#include <string>
#include <vector>

namespace RenderLib {

    enum RenderSystemBackend {
        VULKAN
    };

    std::string GetRenderSystemBackendName(RenderSystemBackend backend);

    struct RenderSystemConfig {
    };

    struct RenderSystemDescriptor {
        RenderSystemBackend backend;
        RenderSystemConfig *config;
    };

    struct RenderSystem {
        RenderSystemBackend backend{};

        virtual ~RenderSystem() = default;
    };

    struct RenderDevice {
        std::string deviceName;
        bool isDiscreteGpu;
        uint64_t capability;
    };

    struct RenderContext {
        RenderSystemBackend backend{};

        virtual ~RenderContext() = default;
    };

    class Window {

    public:
        RenderSystemBackend backend;

        explicit Window(RenderSystemBackend backend);

        virtual void show() = 0;

        virtual void update() = 0;

        virtual void close() = 0;

        virtual bool shouldClose() = 0;
    };

    [[nodiscard]] RenderSystem *CreateRenderSystem(const RenderSystemDescriptor &renderSystemDescriptor);

    /**
     * @brief Get all available render devices
     * @param renderSystem the render system as context
     * @return the list of available render devices
     */
    [[nodiscard]] std::vector<RenderDevice *> GetRenderDevices(RenderSystem *renderSystem);

    /**
     * @brief Returns the best render device which supports the required vulkan features ranked by their capability score
     * @param renderSystem the render system as context
     * @return the render device with the best capability
     */
    [[nodiscard]] RenderDevice *GetBestRenderDevice(RenderSystem *renderSystem);

    /**
     * @brief Create a render context for the specified window with the specified render device
     * @param window the window to create the render context for
     * @param renderSystem the render system as context
     * @param renderDevice the device to use for rendering window contents
     * @return
     */
    [[nodiscard]] RenderContext *
    CreateRenderContext(Window *window, RenderSystem *renderSystem, RenderDevice *renderDevice);

    /**
     *
     * @brief Create a window
     * @param renderContext the context to use for rendering window contents
     * @param title the title of the window
     * @param width the width of the window
     * @param height the height of the window
     * @return the created Window
     */
    Window *CreateNewWindow(const std::string &title, int width, int height);

}