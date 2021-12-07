#include <Dyngine_internal.hpp>
#include <LLGL/LLGL.h>
#include <Dpac/Dpac.hpp>
#include "Shader/ShaderUtil.hpp"
#include "LLGL/Strings.h"
#include <LLGL/Utility.h>

std::shared_ptr<LLGL::RenderSystem> setupRenderSystem() {
    LLGL::RenderSystemDescriptor renderSystemDescriptor{};
#ifdef DYNGINE_USE_VULKAN_API
    // Renderer configuration
    LLGL::RendererConfigurationVulkan vulkanConfig{};
    vulkanConfig.application.applicationName = ENGINE_NAME " " ENGINE_VERSION;
    vulkanConfig.enabledLayers = { "VK_LAYER_LUNARG_standard_validation" };

    renderSystemDescriptor.moduleName = "Vulkan";
    renderSystemDescriptor.rendererConfig = &vulkanConfig;
#endif
#ifdef DYNGINE_USE_DIRECT3D12_API
    // Renderer configuration
    renderSystemDescriptor.moduleName = "Direct3D12";
#endif
#ifdef DYNGINE_USE_DIRECT3D11_API
    // Renderer configuration
    renderSystemDescriptor.moduleName = "Direct3D11";
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
    contextDescriptor.vsync.enabled = true;

    auto renderContext = renderSystem->CreateRenderContext(contextDescriptor);

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

    // Set window title and show window
    auto &window = LLGL::CastTo<LLGL::Window>(renderContext->GetSurface());
    std::wstring windowTitle = L"" ENGINE_NAME " " ENGINE_VERSION;
    window.SetTitle(windowTitle);
    window.Show();

    // Setup Triangle
    LLGL::Buffer *vertexBuffer;
    LLGL::VertexFormat vertexFormat{};
    {
        // Vertex data (3 vertices for our triangle)
        const float s = 0.5f;

        // Vertex data structure
        struct Vertex {
            float position[2];
            uint8_t color[4];
        };

        Vertex vertices[] =
                {
                        {{0,  s},  {255, 0,   0,   255}}, // 1st vertex: center-top, red
                        {{s,  -s}, {0,   255, 0,   255}}, // 2nd vertex: right-bottom, green
                        {{-s, -s}, {0,   0,   255, 255}}, // 3rd vertex: left-bottom, blue
                };

        // Vertex format
        vertexFormat.AppendAttribute({"position", LLGL::Format::RG32Float}); // vertex position
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

    // Setup ShaderProgram
    LLGL::ShaderProgram *shaderProgram;
    {
        LLGL::Shader *vertexShader, *fragmentShader;
        if (ShaderUtil::IsSupported(renderSystem, LLGL::ShadingLanguage::SPIRV)) {
            vertexShader = ShaderUtil::LoadSpirVShader(engineResources, "/triangle.vert.spv",
                                                       renderSystem,
                                                       LLGL::ShaderType::Vertex,
                                                       vertexFormat
            );

            fragmentShader = ShaderUtil::LoadSpirVShader(engineResources,
                                                         "/triangle.hlsl",
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

    // Setup pipeline
    LLGL::PipelineState *pipeline;
    {
        LLGL::GraphicsPipelineDescriptor pipelineDescriptor{
                .shaderProgram = shaderProgram,
                .renderPass = renderContext->GetRenderPass()
        };

        pipeline = renderSystem->CreatePipelineState(pipelineDescriptor);
    }

    LLGL::CommandQueue *queue = renderSystem->GetCommandQueue();

    LLGL::CommandBuffer *mainCmd;
    {
        LLGL::CommandBufferDescriptor commandBufferDescriptor{
                .flags =(LLGL::CommandBufferFlags::MultiSubmit)
        };
        mainCmd = renderSystem->CreateCommandBuffer(commandBufferDescriptor);
    }

    // Main loop
    while (window.ProcessEvents()) {
        // Record commands
        {
            mainCmd->Begin();
            mainCmd->SetPipelineState(*pipeline);
            {
                mainCmd->SetClearColor(LLGL::ColorRGBAf(0.1f, 0.1f, 0.1f));
                mainCmd->SetVertexBuffer(*vertexBuffer);
                mainCmd->BeginRenderPass(*renderContext);
                {
                    mainCmd->Clear(LLGL::ClearFlags::ColorDepth);
                    mainCmd->SetViewport(renderContext->GetResolution());
                    // Draw triangle with 3 vertices
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
