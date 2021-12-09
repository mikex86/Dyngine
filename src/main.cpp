#include <RenderLib/RenderLib.hpp>
#include <RenderLib/ContextManagementVulkan.hpp>
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
    auto renderSystem = std::unique_ptr<RenderLib::RenderSystem>(RenderLib::CreateRenderSystem(renderSystemDescriptor));
    auto renderDevice = std::unique_ptr<RenderLib::RenderDevice>(RenderLib::GetBestRenderDevice(renderSystem.get()));

    std::cout << "Render device: " << renderDevice->deviceName << ", Is discrete GPU: "
              << (renderDevice->isDiscreteGpu ? "true" : "false")
              << ", Capability: " << renderDevice->capability << std::endl;

    auto window = RenderLib::CreateNewWindow(ENGINE_NAME " " ENGINE_VERSION, 800, 600);

    auto renderContext = std::unique_ptr<RenderLib::RenderContext>(
            RenderLib::CreateRenderContext(window, renderSystem.get(), renderDevice.get())
    );

    auto vertexShader = std::unique_ptr<RenderLib::Shader>(LoadPrecompiledShaderFromArchive(
            renderContext.get(),
            engineResources,
            "/triangle.frag.glsl.spv",
            RenderLib::ShaderType::VERTEX_SHADER
    ));

    auto fragmentShader = std::unique_ptr<RenderLib::Shader>(LoadPrecompiledShaderFromArchive(
            renderContext.get(),
            engineResources,
            "/triangle.frag.glsl.spv",
            RenderLib::ShaderType::FRAGMENT_SHADER
    ));

    auto shaderProgram = std::unique_ptr<RenderLib::ShaderProgram>(RenderLib::CreateShaderProgram(renderContext.get(), {
            {RenderLib::VERTEX_SHADER,   vertexShader.get()},
            {RenderLib::FRAGMENT_SHADER, fragmentShader.get()}
    }));

    RenderLib::VertexFormat vertexFormat{};
    {
        vertexFormat.appendAttribute("position", RenderLib::VertexAttributeType::FLOAT_VEC2);
        vertexFormat.appendAttribute("color", RenderLib::VertexAttributeType::UBYTE_VEC4);
    }

    auto pipelineLayout = RenderLib::PipelineLayout{
            .uniformDescriptors = {
                    {
                            .elementCount = 1,
                            .acceptingShaderTypes = {
                                    RenderLib::ShaderType::FRAGMENT_SHADER
                            }
                    }
            }
    };
    auto graphicsPipeline = RenderLib::CreateGraphicsPipeline(
            renderContext.get(),
            vertexFormat,
            pipelineLayout,
            std::move(shaderProgram)
    );

    window->show();

    while (!window->shouldClose()) {
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