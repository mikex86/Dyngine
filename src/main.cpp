#include <Dyngine_internal.hpp>
#include <LLGL/LLGL.h>
#include <Dpac/Dpac.hpp>
#include "Shader/ShaderUtil.hpp"
#include "LLGL/Strings.h"
#include "Camera/PerspectiveCamera.hpp"
#include <LLGL/Utility.h>

std::shared_ptr<LLGL::RenderSystem> setupRenderSystem() {
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
    dpac::ReadOnlyArchive engineResources = dpac::ReadOnlyArchive::Open("EngineResources.dpac");

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

    // Setup Triangle
    LLGL::Buffer *vertexBuffer;
    LLGL::VertexFormat vertexFormat{};
    {
        // Vertex data (3 vertices for our triangle)
        const float s = 0.5f;

        // Vertex data structure
        struct Vertex {
            float position[3];
            uint8_t color[4];
        };

        Vertex vertices[] =
                {
                        {{0,  s,  0}, {255, 0,   0,   255}}, // 1st vertex: center-top, red
                        {{s,  -s, 0}, {0,   255, 0,   255}}, // 2nd vertex: right-bottom, green
                        {{-s, -s, 0}, {0,   0,   255, 255}}, // 3rd vertex: left-bottom, blue
                };

        // Vertex format
        vertexFormat.AppendAttribute({"position", LLGL::Format::RGB32Float}); // vertex position
        vertexFormat.AppendAttribute({"color", LLGL::Format::RGBA8UNorm}); // color
        vertexFormat.SetStride(sizeof(Vertex));

        // Vertex Buffer
        LLGL::BufferDescriptor vertexBufferDesc{
                .size =sizeof(vertices),
                .bindFlags = LLGL::BindFlags::VertexBuffer,
                .vertexAttribs = vertexFormat.attributes
        };
        vertexBuffer = renderSystem->CreateBuffer(vertexBufferDesc, vertices);
    }

    auto windowSize = window->GetContentSize();

    auto aspectRatio = static_cast<float>(windowSize.width) / static_cast<float>(windowSize.height);

    PerspectiveCamera camera(70, aspectRatio, 0.1f,
                             100.0f);

    camera.setPosition({0, 0, 5});

    // Setup ShaderProgram
    LLGL::ShaderProgram *shaderProgram;
    {
        LLGL::Shader *vertexShader, *fragmentShader;
        if (ShaderUtil::IsSupported(renderSystem, LLGL::ShadingLanguage::SPIRV)) {
            vertexShader = ShaderUtil::LoadSpirVShader(engineResources, "/triangle.vert.glsl.spv",
                                                       renderSystem,
                                                       LLGL::ShaderType::Vertex,
                                                       vertexFormat
            );

            fragmentShader = ShaderUtil::LoadSpirVShader(engineResources,
                                                         "/triangle.frag.glsl.spv",
                                                         renderSystem,
                                                         LLGL::ShaderType::Fragment,
                                                         vertexFormat
            );
        } else {
            vertexShader = ShaderUtil::LoadHLSLShader(engineResources,
                                                      "/triangle.vert.hlsl.cso",
                                                      renderSystem,
                                                      LLGL::ShaderType::Vertex,
                                                      vertexFormat,
                                                      "VS",
                                                      "vs_4_0"
            );

            fragmentShader = ShaderUtil::LoadHLSLShader(engineResources,
                                                        "/triangle.frag.hlsl.cso",
                                                        renderSystem,
                                                        LLGL::ShaderType::Fragment,
                                                        vertexFormat,
                                                        "PS",
                                                        "ps_4_0"
            );
        }

        shaderProgram = ShaderUtil::CreateShaderProgram(
                renderSystem,
                vertexShader,
                fragmentShader
        );
    }

    // Create CameraShaderStateBuffer Buffer
    LLGL::Buffer *cameraShaderStateBuffer;
    {
        cameraShaderStateBuffer = renderSystem->CreateBuffer(
                LLGL::ConstantBufferDesc(
                        sizeof(CameraShaderState)
                ),
                &camera.getCameraShaderState()
        );
    }

    // Setup pipeline
    LLGL::PipelineLayout *pipelineLayout = renderSystem->CreatePipelineLayout(
            LLGL::PipelineLayoutDesc("cbuffer(CameraShaderState@1):frag:vert")
    );

    LLGL::PipelineState *pipeline;
    {
        LLGL::GraphicsPipelineDescriptor pipelineDescriptor{
                .pipelineLayout = pipelineLayout,
                .shaderProgram = shaderProgram,
                .renderPass = renderContext->GetRenderPass(),
        };

        pipeline = renderSystem->CreatePipelineState(pipelineDescriptor);
    }

    // Create ResourceHeap
    LLGL::ResourceHeap *resourceHeap;
    {
        LLGL::ResourceHeapDescriptor resourceHeapDesc{
                .pipelineLayout = pipelineLayout,
                .resourceViews = {cameraShaderStateBuffer}
        };
        resourceHeap = renderSystem->CreateResourceHeap(resourceHeapDesc);
    }

    LLGL::CommandQueue *queue = renderSystem->GetCommandQueue();

    LLGL::CommandBuffer *mainCmd;
    {
        LLGL::CommandBufferDescriptor commandBufferDescriptor{
                .flags =(LLGL::CommandBufferFlags::MultiSubmit)
        };
        mainCmd = renderSystem->CreateCommandBuffer(commandBufferDescriptor);
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

    std::unique_ptr<LLGL::Display> display = window->FindResidentDisplay();

    // Main loop
    while (window->ProcessEvents()) {
        frameTimer->MeasureTime();
        float moveForward = 0;
        float moveStrafing = 0;
        float moveUp = 0;
        {
            bool wPressed = input->KeyPressed(LLGL::Key::W);
            bool sPressed = input->KeyPressed(LLGL::Key::S);
            bool aPressed = input->KeyPressed(LLGL::Key::A);
            bool dPressed = input->KeyPressed(LLGL::Key::D);
            bool spacePressed = input->KeyPressed(LLGL::Key::Space);
            bool shiftPressed = input->KeyPressed(LLGL::Key::Shift);
            if (wPressed ^ sPressed) {
                if (wPressed)
                    moveForward = 1;
                if (sPressed)
                    moveForward = -1;
            }
            if (aPressed ^ dPressed) {
                if (aPressed)
                    moveStrafing = 1;
                if (dPressed)
                    moveStrafing = -1;
            }
            if (spacePressed ^ shiftPressed) {
                if (spacePressed)
                    moveUp = 1;
                if (shiftPressed)
                    moveUp = -1;
            }
        }
        windowSize = window->GetSize();
        auto windowPosition = window->GetPosition();

        if (window->HasFocus()) {
            display->ShowCursor(false);
            display->SetCursorPosition(static_cast<int>(windowPosition.x + windowSize.width / 2),
                                       static_cast<int>(windowPosition.y + windowSize.height / 2));
            auto mouseMotion = input->GetMouseMotion();
            camera.setYaw(camera.getYaw() + static_cast<float>(mouseMotion.x) * 0.1f);
            camera.setPitch(camera.getPitch() + static_cast<float>(mouseMotion.y) * -0.1f);
        } else {
            display->ShowCursor(true);
        }

        if (moveStrafing != 0 || moveForward != 0 || moveUp != 0) {
            auto deltaTime = static_cast<float>(frameTimer->GetDeltaTime());
            auto direction = camera.getDirection() * moveForward + camera.getCameraRight() * moveStrafing +
                             camera.getCameraUp() * moveUp;
            auto speed = 2.0f * deltaTime;
            camera.setPosition(camera.getPosition() + direction * speed);
        }

        if (windowSize.width != 0 && windowSize.height != 0) {
            auto newAspectRatio = static_cast<float>(windowSize.width) / static_cast<float>(windowSize.height);
            if (newAspectRatio != aspectRatio) {
                camera.setAspect(newAspectRatio);
                aspectRatio = newAspectRatio;
            }
        }

        auto videoMode = renderContext->GetVideoMode();
        videoMode.resolution = windowSize;
        renderContext->SetVideoMode(videoMode);

        auto hasChanged = camera.update();

        // Record commands
        if (hasChanged) {
            mainCmd->Begin();
            mainCmd->SetPipelineState(*pipeline);
            mainCmd->SetResourceHeap(*resourceHeap);

            {
                mainCmd->SetClearColor(LLGL::ColorRGBAf(0.1f, 0.1f, 0.1f));
                mainCmd->BeginRenderPass(*renderContext);
                {
                    mainCmd->Clear(LLGL::ClearFlags::ColorDepth);
                    mainCmd->SetViewport(window->GetSize());
                    mainCmd->SetVertexBuffer(*vertexBuffer);
                    mainCmd->UpdateBuffer(*cameraShaderStateBuffer,
                                          0,
                                          &camera.getCameraShaderState(),
                                          sizeof(CameraShaderState)
                    );
                    mainCmd->Draw(3, 0);
                }
                mainCmd->EndRenderPass();
            }
            mainCmd->End();
        }
        queue->Submit(*mainCmd);
        renderContext->Present();
    }
}

int main() {
    try {
        RunEngine();
    } catch (std::exception &e) {
        std::cerr << "RunEngine() failed exceptionally: " << e.what() << std::endl;
    }
}
