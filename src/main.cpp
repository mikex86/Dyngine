#include <Dyngine_internal.h>
#include <LLGL/LLGL.h>
#include <ErrorHandling/IllegalStateException.hpp>
#include <Dpac/Dpac.hpp>

// Vertex data structure
struct Vertex {
    float position[2];
    uint8_t color[4];
};

void RunEngine() {
    // Read Engine Resources
    auto engineResources = dpac::ReadOnlyArchive::Open("EngineResources.dpac");

    // Renderer configuration
    LLGL::RendererConfigurationVulkan vulkanConfig{};
    vulkanConfig.application.applicationName = ENGINE_NAME " " ENGINE_VERSION;

    LLGL::RenderSystemDescriptor renderSystemDescriptor{};
    renderSystemDescriptor.moduleName = "Vulkan";
    renderSystemDescriptor.rendererConfig = &vulkanConfig;

    // Load render system module
    auto renderSystem = LLGL::RenderSystem::Load(renderSystemDescriptor);

    // Create render context
    LLGL::RenderContextDescriptor contextDescriptor{};
    contextDescriptor.videoMode.resolution = {800, 600};
    contextDescriptor.vsync.enabled = true;
    contextDescriptor.samples = 8;

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
    {
        // Vertex data (3 vertices for our triangle)
        const float s = 0.5f;

        Vertex vertices[] =
                {
                        {{0,  s},  {255, 0,   0,   255}}, // 1st vertex: center-top, red
                        {{s,  -s}, {0,   255, 0,   255}}, // 2nd vertex: right-bottom, green
                        {{-s, -s}, {0,   0,   255, 255}}, // 3rd vertex: left-bottom, blue
                };

        // Vertex format
        LLGL::VertexFormat vertexFormat{};
        vertexFormat.AppendAttribute({"position", LLGL::Format::RG32Float}); // vertex position
        vertexFormat.AppendAttribute({"color", LLGL::Format::RGBA8UNorm}); // color
        vertexFormat.SetStride(sizeof(Vertex));

        // Vertex Buffer
        LLGL::BufferDescriptor bufferDescriptor{};
        bufferDescriptor.size = sizeof(vertices);
        bufferDescriptor.bindFlags = LLGL::BindFlags::VertexBuffer;
        bufferDescriptor.vertexAttribs = vertexFormat.attributes;

        LLGL::Buffer *vertexBuffer = renderSystem->CreateBuffer(bufferDescriptor);

        LLGL::Shader *vertShader{};

        const auto &languages = renderSystem->GetRenderingCaps().shadingLanguages;
        if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::SPIRV) == languages.end()) {
            RAISE_EXCEPTION(errorhandling::IllegalStateException, "Spir-V shading language not available");
        }
        char *vertexShaderContent;
        uint64_t shaderContentSize;
        {
            auto vertexShaderStream = engineResources.getEntryStream("/triangle.vert.spv");
            shaderContentSize = vertexShaderStream.getSize();
            vertexShaderContent = new char[shaderContentSize];

            for (uint64_t i = 0; i < shaderContentSize; i++) {
                char byte = vertexShaderStream.readInt8();
                vertexShaderContent[i] = byte;
            }
        }
        LLGL::ShaderDescriptor vertShaderDesc = {
                LLGL::ShaderType::Vertex,
                vertexShaderContent,
                nullptr,
                nullptr
        };
        vertShaderDesc.sourceType = LLGL::ShaderSourceType::BinaryBuffer;
        vertShaderDesc.sourceSize = shaderContentSize;
        vertShader = renderSystem->CreateShader(vertShaderDesc);

    }

    // Main loop
    while (window.ProcessEvents()) {
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
