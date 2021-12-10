#include <RenderLib/RenderLib.hpp>
#include <RenderLib/RenderContextVulkan.hpp>
#include <Dyngine_internal.hpp>
#include <Shader/ShaderUtil.hpp>
#include <iostream>
#include <memory>

int RunEngine() {
    auto engineResources = dpac::ReadOnlyArchive::Open("EngineResources.dpac");
    RenderLib::VulkanRenderSystemConfig renderSystemConfig{
            .application = {
                    .applicationName = "DyngineApplication",
                    .applicationVersionMajor = 1,
                    .applicationVersionMinor = 0,
                    .applicationVersionPatch = 0,
                    .engineName = "Dyngine",
                    .engineVersionMajor = 1,
                    .engineVersionMinor = 0,
                    .engineVersionPatch = 0,
            },
            .instanceLayers = {
                    "VK_LAYER_KHRONOS_validation",
            }
    };
    RenderLib::RenderSystemDescriptor renderSystemDescriptor{
            .backend = RenderLib::RenderSystemBackend::VULKAN,
            .config = &renderSystemConfig
    };
    auto renderSystem = RenderLib::CreateRenderSystem(renderSystemDescriptor);
    auto renderDevice = RenderLib::GetBestRenderDevice(renderSystem);

    std::cout << "Render device: " << renderDevice->deviceName << ", Is discrete GPU: "
              << (renderDevice->isDiscreteGpu ? "true" : "false")
              << ", Capability: " << renderDevice->capability << std::endl;

    auto window = RenderLib::CreateNewWindow(ENGINE_NAME " " ENGINE_VERSION, 800, 600);

    auto renderContext = RenderLib::CreateRenderContext(window, renderSystem, renderDevice);

    auto vertexShader = LoadPrecompiledShaderFromArchive(
            renderContext,
            engineResources,
            "/triangle.vert.glsl.spv",
            RenderLib::ShaderType::VERTEX_SHADER
    );

    auto fragmentShader = LoadPrecompiledShaderFromArchive(
            renderContext,
            engineResources,
            "/triangle.frag.glsl.spv",
            RenderLib::ShaderType::FRAGMENT_SHADER
    );

    auto shaderProgram = RenderLib::CreateShaderProgram(renderContext, {
            {RenderLib::VERTEX_SHADER,   vertexShader.get()},
            {RenderLib::FRAGMENT_SHADER, fragmentShader.get()}
    });

    RenderLib::VertexFormat vertexFormat{};
    {
        vertexFormat.appendAttribute("position", RenderLib::VertexAttributeType::FLOAT_VEC2);
        vertexFormat.appendAttribute("color", RenderLib::VertexAttributeType::UBYTE_VEC4);
    }

    auto pipelineLayout = RenderLib::PipelineLayout{};
    auto graphicsPipeline = RenderLib::CreateGraphicsPipeline(
            renderContext,
            vertexFormat,
            pipelineLayout,
            shaderProgram
    );

    auto frameBuffer = RenderLib::CreateFrameBuffer(renderContext);
    auto commandBuffer = RenderLib::CreateCommandBuffer(frameBuffer);

    window->show();

    while (!window->shouldClose()) {
        renderContext->beginFrame();
        {
            commandBuffer->begin();
            {
                commandBuffer->beginRenderPass();
                commandBuffer->bindGraphicsPipeline(graphicsPipeline);
                commandBuffer->endRenderPass();
            }
            commandBuffer->end();
        }
        renderContext->drawFrame(commandBuffer);
        renderContext->endFrame();
        renderContext->synchronize();
        window->update();
    }

    window->close();
    return 0;
}

int main() {
    try {
        return RunEngine();
    } catch (const std::exception &e) {
        std::cerr << "Failed to run engine: " << e.what() << std::endl;
        return 1;
    }
}