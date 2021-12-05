#include <Dyngine_internal.h>
#include <LLGL/LLGL.h>
#include <Dpac/Dpac.hpp>
#include "Shader/ShaderUtil.hpp"
#include <LLGL/Utility.h>

void RunEngine() {
    // Read Engine Resources
    auto engineResources = dpac::ReadOnlyArchive::Open("EngineResources.dpac");

    // Renderer configuration
    LLGL::RendererConfigurationVulkan vulkanConfig{};
    vulkanConfig.application.applicationName = ENGINE_NAME " " ENGINE_VERSION;
    vulkanConfig.enabledLayers = { "VK_LAYER_LUNARG_standard_validation" };

    LLGL::RenderSystemDescriptor renderSystemDescriptor{};
    renderSystemDescriptor.moduleName = "Vulkan";
    renderSystemDescriptor.rendererConfig = &vulkanConfig;

    // Load render system module
    std::shared_ptr<LLGL::RenderSystem> renderSystem = LLGL::RenderSystem::Load(renderSystemDescriptor);

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
    std::cout << "Shading-Language: " << renderInfo.shadingLanguageName << std::endl;

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
        LLGL::Shader *vertexShader = ShaderUtil::LoadSpirVShader(engineResources, "/triangle.vert.spv",
                                                                 renderSystem,
                                                                 LLGL::ShaderType::Vertex,
                                                                 vertexFormat
        );

        LLGL::Shader *fragmentShader = ShaderUtil::LoadSpirVShader(engineResources,
                                                                   "/triangle.frag.spv",
                                                                   renderSystem,
                                                                   LLGL::ShaderType::Fragment,
                                                                   vertexFormat
        );

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
