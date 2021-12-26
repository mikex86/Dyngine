#include <Dyngine_internal.hpp>
#include <LLGL/LLGL.h>
#include <LLGL/Strings.h>
#include <LLGL/Utility.h>
#include <Dpac/Dpac.hpp>
#include "Rendering/Shader/ShaderUtil.hpp"
#include "Rendering/Camera/PerspectiveCamera.hpp"
#include "Rendering/Camera/Controller/FlyingPerspectiveCameraController.hpp"
#include "Rendering/Asset/AssetLoader.hpp"
#include "Rendering/Shader/ShaderCache.hpp"
#include "Rendering/Asset/AssetRenderer.hpp"

std::unique_ptr<LLGL::RenderSystem> setupRenderSystem() {
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
    renderSystemDescriptor.moduleName = "OpenGL";
#endif
    return LLGL::RenderSystem::Load(renderSystemDescriptor);
}

void RunEngine() {
    // Read Engine Resources
    Dpac::ReadOnlyArchive engineResources = Dpac::ReadOnlyArchive::Open("EngineResources.dpac");

    // Load render system module
    std::shared_ptr<LLGL::RenderSystem> renderSystem = setupRenderSystem();

    // Create render context
    LLGL::RenderContextDescriptor contextDescriptor{};
    contextDescriptor.videoMode.resolution = {800, 600};
    contextDescriptor.vsync.enabled = false;

    auto input = std::make_shared<LLGL::Input>();

    LLGL::WindowDescriptor windowDescriptor{
            .title = L"" ENGINE_NAME " " ENGINE_VERSION,
            .size = {800, 600},
            .borderless = false,
            .resizable = true,
            .centered = true,
    };

    auto window = std::shared_ptr<LLGL::Window>(std::move(LLGL::Window::Create(windowDescriptor)));
    window->AddEventListener(input);

    auto renderContext = renderSystem->CreateRenderContext(contextDescriptor, window);


    auto windowSize = window->GetSize();

    auto aspectRatio = static_cast<float>(windowSize.width) / static_cast<float>(windowSize.height);

    PerspectiveCamera camera(70, aspectRatio, 0.1f,
                             100.0f);

    camera.setPosition({0, 0, 5});

    ShaderCache shaderCache(renderSystem, engineResources);

    auto asset = std::unique_ptr<Asset>(
            AssetLoader::LoadAsset(renderSystem, engineResources.getEntryStream("/BuddyDroid_01DMG_rig.dasset")));
    std::shared_ptr<AssetRenderer> assetRenderer = std::shared_ptr<AssetRenderer>(
            AssetRenderer::fromAsset(renderSystem, renderContext, shaderCache, asset, camera)
    );

//    std::shared_ptr<MeshRenderer> meshRenderer1 = std::shared_ptr<MeshRenderer>(
//            MeshRenderer::newTriangleMeshRenderer(renderSystem, renderContext, shaderCache, camera, {0, 0, 0})
//    );
    LLGL::CommandQueue *queue = renderSystem->GetCommandQueue();

    LLGL::CommandBuffer *commandBuffer;
    {
        LLGL::CommandBufferDescriptor commandBufferDescriptor{
                .flags =(LLGL::CommandBufferFlags::MultiSubmit)
        };
        commandBuffer = renderSystem->CreateCommandBuffer(commandBufferDescriptor);
    }

    window->Show();

    // Print renderer information
    auto &renderInfo = renderSystem->GetRendererInfo();
    std::cout << "Renderer: " << renderInfo.rendererName << std::endl;
    std::cout << "Device: " << renderInfo.deviceName << std::endl;
    std::cout << "Vendor: " << renderInfo.vendorName << std::endl;
    const auto &supportedShadingLanguages = renderSystem->GetRenderingCaps().shadingLanguages;
    std::cout << "Shading-Languages: ";
    for (const auto &shadingLanguage: supportedShadingLanguages) {
        std::cout << LLGL::ToString(shadingLanguage) << " ";
    }
    std::cout << std::endl;

    std::unique_ptr<LLGL::Timer> frameTimer = LLGL::Timer::Create();

    std::shared_ptr<LLGL::Display> display = window->FindResidentDisplay();

    FlyingPerspectiveCameraController cameraController(camera, input, display, window);

    // Main loop
    while (window->ProcessEvents()) {

        windowSize = window->GetSize();

        // Update context resolution
        auto videoMode = renderContext->GetVideoMode();
        videoMode.resolution = windowSize;
        renderContext->SetVideoMode(videoMode);

        // Update aspect ratio
        if (windowSize.width != 0 && windowSize.height != 0) {
            auto newAspectRatio = static_cast<float>(windowSize.width) / static_cast<float>(windowSize.height);
            if (newAspectRatio != aspectRatio) {
                camera.setAspect(newAspectRatio);
                aspectRatio = newAspectRatio;
            }
        }

        // Update camera
        cameraController.update(static_cast<float>(frameTimer->GetDeltaTime()));

        auto hasChanged = camera.update();

        // Record commands
        if (hasChanged) {
            commandBuffer->Begin();
            {
                commandBuffer->BeginRenderPass(*renderContext);
                {
                    commandBuffer->Clear(LLGL::ClearFlags::ColorDepth);
                    commandBuffer->SetViewport(window->GetSize());
                    assetRenderer->render(*commandBuffer);
                }
                commandBuffer->EndRenderPass();
            }
            commandBuffer->End();
        }
        queue->Submit(*commandBuffer);
        renderContext->Present();
        frameTimer->MeasureTime();
    }
}

int main() {
//    try {
    RunEngine();
//    } catch (std::exception &e) {
//        std::cerr << "RunEngine() failed exceptionally: " << e.what() << std::endl;
//    }
}
