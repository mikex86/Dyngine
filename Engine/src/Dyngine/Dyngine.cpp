#include <LLGL/LLGL.h>
#include <LLGL/Strings.h>
#include <LLGL/Utility.h>
#include <Dpac/Dpac.hpp>
#include "Dyngine/Dyngine.hpp"
#include "Dyngine/Input/Input.hpp"
#include "Dyngine/EngineState.hpp"
#include "Dyngine/Rendering/Shader/ShaderUtil.hpp"
#include "Dyngine/Rendering/Scene/Camera/PerspectiveCamera.hpp"
#include "Dyngine/Rendering/Scene/Camera/Controller/FlyingPerspectiveCameraController.hpp"
#include "Dyngine/Rendering/Scene/Asset/AssetLoader.hpp"
#include "Dyngine/Rendering/Shader/ShaderCache.hpp"
#include "Dyngine/Rendering/Scene/Asset/AssetRenderer.hpp"
#include "Dyngine/Rendering/Scene/Scene.hpp"
#include "Dyngine/Rendering/Scene/SceneRenderer.hpp"
#include "Dyngine/EngineRenderContextState.hpp"
#include "ErrorHandling/IllegalArgumentException.hpp"


namespace Dyngine {

    EngineInstance::EngineInstance(const std::shared_ptr<EngineRenderTarget> &engineRenderTarget,
                                   const std::shared_ptr<InputProvider> &inputProvider) {

        engineState = new EngineState(
                engineRenderTarget,
                inputProvider
        );
    }

    void EngineInstance::startEngine() {
        auto renderContextState = engineState->engineRenderTarget->getRenderContext()->getRenderContextState();
        auto &renderSystem = renderContextState->renderSystem;

        // Read Engine Resources
        Dpac::ReadOnlyArchive engineResources = Dpac::ReadOnlyArchive::Open("EngineResources.dpac");

        auto renderContext = renderContextState->renderTarget;
        auto resolution = renderContext->GetResolution();

        auto aspectRatio =
                static_cast<float>(resolution.width) / static_cast<float>(resolution.height);

        engineState->camera = std::make_unique<PerspectiveCamera>(70.0f, aspectRatio, 0.001f,
                                                                  100.0f);

        engineState->camera->setPosition({0, 0.25f, 1});

        std::unique_ptr<ShaderCache> shaderCache = std::make_unique<ShaderCache>(renderSystem, engineResources);

        std::unique_ptr<Scene> scene = std::make_unique<Scene>(*engineState->camera);

        scene->addLight(Light(LightType::POINT, {0, 1, 0}, glm::vec3{1, 0, 0}, 1.0f));
        scene->addLight(Light(LightType::POINT, {1, 0, 0}, glm::vec3{1, 1, 1}, 1.0f));

        {
            auto asset = std::unique_ptr<Asset>(
                    AssetLoader::LoadAsset(renderSystem,
                                           engineResources.getEntryStream("/BuddyDroid_01DMG_rig.dasset")));
            scene->addAsset(asset);
        }

        engineState->sceneRenderer = std::make_unique<SceneRenderer>(renderSystem, renderContextState->renderTarget,
                                                                     std::move(shaderCache), std::move(scene));

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

        auto input = std::make_unique<Input>(engineState->inputProvider);
        engineState->cameraController = std::make_unique<FlyingPerspectiveCameraController>(*engineState->camera,
                                                                                            input);
    }

    void EngineInstance::renderFrame() {
        auto renderContextState = engineState->engineRenderTarget->getRenderContext()->getRenderContextState();
        // Update context resolution
        auto renderTarget = renderContextState->renderTarget;

        auto resolution = renderTarget->GetResolution();

        // Update aspect ratio
        if (resolution.width != 0 && resolution.height != 0) {
            auto newAspectRatio = static_cast<float>(resolution.width) / static_cast<float>(resolution.height);
            if (newAspectRatio != engineState->aspectRatio) {
                engineState->camera->setAspect(newAspectRatio);
                engineState->aspectRatio = newAspectRatio;
            }
        }

        // Update camera
        engineState->cameraController->update(static_cast<float>(engineState->frameTimer->GetDeltaTime()));

        auto hasChanged = engineState->camera->update();
        auto &commandBuffer = renderContextState->commandBuffer;

        // Record commands
        if (hasChanged) {
            commandBuffer->Begin();
            {
                commandBuffer->BeginRenderPass(*renderTarget);
                {
                    commandBuffer->SetClearColor({0.0f, 0.0f, 0.0f, 1.0f});
                    commandBuffer->Clear(LLGL::ClearFlags::ColorDepth);
                    commandBuffer->SetViewport(resolution);
                    engineState->sceneRenderer->render(*commandBuffer);
                }
                commandBuffer->EndRenderPass();
            }
            commandBuffer->End();
        }
        LLGL::CommandQueue *queue = renderContextState->renderSystem->GetCommandQueue();
        queue->Submit(*commandBuffer);
        renderContextState->renderContext->Present();

        engineState->frameTimer->MeasureTime();
    }

    EngineInstance::~EngineInstance() {
        delete engineState;
    }

    int EngineInstance::getFPS() {
        return engineState->frameTimer->GetFrequency();
    }

}