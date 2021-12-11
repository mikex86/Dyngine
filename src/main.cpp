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

    std::shared_ptr<RenderLib::BufferObject> vertexBufferObject;
    std::shared_ptr<RenderLib::BufferObject> indexBufferObject;

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

    RenderLib::BufferDescriptor bufferDescriptor(
            sizeof(vertices),
            RenderLib::BufferType::VERTEX_BUFFER
    );
    vertexBufferObject = RenderLib::CreateBufferObject(renderContext, bufferDescriptor, vertices, sizeof(vertices));

    uint32_t indices[] = {0, 1, 2};
    RenderLib::BufferDescriptor indexBufferDescriptor(
            sizeof(indices),
            RenderLib::BufferType::INDEX_BUFFER
    );
    indexBufferObject = RenderLib::CreateBufferObject(renderContext, indexBufferDescriptor, indices, sizeof(indices));


    auto pipelineLayout = RenderLib::PipelineLayout{};

    auto graphicsPipeline = RenderLib::CreateGraphicsPipeline(
            renderContext,
            vertexFormat,
            pipelineLayout,
            shaderProgram
    );

    auto commandBuffer = RenderLib::CreateCommandBuffer(renderContext);

    window->show();

    try {
        while (!window->shouldClose()) {
            renderContext->beginFrame();
            {
                commandBuffer->begin();
                {
                    commandBuffer->beginRenderPass();
                    commandBuffer->bindGraphicsPipeline(graphicsPipeline);
                    commandBuffer->bindBufferObject(vertexBufferObject);
                    commandBuffer->bindBufferObject(indexBufferObject);
                    commandBuffer->draw(3, 1, 0, 0);
                    commandBuffer->endRenderPass();
                }
                commandBuffer->end();
            }
            renderContext->drawFrame(commandBuffer);
            renderContext->endFrame();
            renderContext->synchronize();
            window->update();
        }
    } catch (const std::exception &e) {
        renderContext->synchronize();
        throw e;
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