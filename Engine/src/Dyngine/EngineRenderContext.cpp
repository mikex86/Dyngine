#include "Dyngine/EngineRenderContext.hpp"
#include "Dyngine/EngineRenderContextState.hpp"
#include <LLGL/LLGL.h>

Dyngine::EngineRenderContext::EngineRenderContext() {
    LLGL::RenderSystemDescriptor renderSystemDescriptor{};
#ifdef DYNGINE_USE_VULKAN_API
    // Renderer configuration
        LLGL::RendererConfigurationVulkan vulkanConfig{};
        vulkanConfig.application.applicationName = ENGINE_NAME " " ENGINE_VERSION;
        vulkanConfig.enabledLayers = {"VK_LAYER_LUNARG_standard_validation"};

        renderSystemDescriptor.moduleName = "Vulkan";
        renderSystemDescriptor.rendererConfig = &vulkanConfig;
#endif
#ifdef DYNGINE_USE_DIRECT3D12_API
    renderSystemDescriptor.moduleName = "Direct3D12";
#endif
#ifdef DYNGINE_USE_DIRECT3D11_API
    renderSystemDescriptor.moduleName = "Direct3D11";
#endif
#ifdef DYNGINE_USE_OPENGL_API
    LLGL::RendererConfigurationOpenGL openGLRenderConfig{
            .contextProfile = LLGL::OpenGLContextProfile::CoreProfile
    };
    renderSystemDescriptor.moduleName = "OpenGL";
    renderSystemDescriptor.rendererConfig = &openGLRenderConfig;
    renderSystemDescriptor.rendererConfigSize = sizeof(LLGL::RendererConfigurationOpenGL);
#endif
    renderContextState = new EngineRenderContextState(LLGL::RenderSystem::Load(renderSystemDescriptor));

}

Dyngine::EngineRenderContextState *Dyngine::EngineRenderContext::getRenderContextState() {
    return renderContextState;
}

Dyngine::EngineRenderContext::~EngineRenderContext() {
    delete renderContextState;
}
